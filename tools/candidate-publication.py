#!/usr/bin/env python3
"""Guard publication of the exact prevalidated PR #5 images to candidate tags only."""

import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import urllib.error
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
LOCK = json.loads(Path(__file__).with_name("candidate-images-20260930.json").read_text())
ACCEPT = ", ".join(
    [
        "application/vnd.oci.image.index.v1+json",
        "application/vnd.oci.image.manifest.v1+json",
        "application/vnd.docker.distribution.manifest.list.v2+json",
        "application/vnd.docker.distribution.manifest.v2+json",
    ]
)
sys.path.insert(0, str(ROOT / "extras/ci"))
from image_smoke import fatal_log_lines  # noqa: E402


def read(path):
    return json.loads(path.read_text())


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def docker(*args):
    return subprocess.check_output(["docker", *args], text=True).strip()


def request_json(url, headers=None):
    with urllib.request.urlopen(
        urllib.request.Request(url, headers=headers or {}), timeout=60
    ) as r:
        return json.load(r), dict(r.headers)


def registry_manifest(reference, token):
    url = f"https://registry-1.docker.io/v2/{LOCK['repository']}/manifests/{reference}"
    request = urllib.request.Request(
        url, headers={"Authorization": f"Bearer {token}", "Accept": ACCEPT}
    )
    with urllib.request.urlopen(request, timeout=60) as response:
        raw = response.read()
        calculated = "sha256:" + hashlib.sha256(raw).hexdigest()
        require(
            response.headers.get("Docker-Content-Digest") == calculated,
            "Registry manifest digest mismatch",
        )
        return json.loads(raw), calculated


def registry_image(arch, allow_missing=False):
    """Anonymous registry read; do not print or persist the short-lived pull token."""
    target = LOCK["images"][arch]
    require(
        re.fullmatch(r"validation-pr5-abad9d70-(arm64-rdp|amd64)", target["tag"]),
        "Not a candidate tag",
    )
    auth, _ = request_json(
        "https://auth.docker.io/token?"
        + urllib.parse.urlencode(
            {
                "service": "registry.docker.io",
                "scope": f"repository:{LOCK['repository']}:pull",
            }
        )
    )
    token = auth["token"]
    try:
        manifest, top_digest = registry_manifest(target["tag"], token)
    except urllib.error.HTTPError as error:
        if error.code == 404 and allow_missing:
            return {"exists": False, "tag": target["tag"]}
        raise
    platform_digest = top_digest
    if "manifests" in manifest:
        selected = [
            m
            for m in manifest["manifests"]
            if m.get("platform", {}).get("os") == "linux"
            and m["platform"].get("architecture") == arch
        ]
        require(len(selected) == 1, "Missing or ambiguous platform in registry index")
        platform_digest = selected[0]["digest"]
        manifest, actual = registry_manifest(platform_digest, token)
        require(actual == platform_digest, "Platform manifest digest mismatch")
    config_digest = manifest.get("config", {}).get("digest", "")
    require(re.fullmatch(r"sha256:[0-9a-f]{64}", config_digest), "Invalid config digest")
    # Docker's containerd store reports the index/manifest digest as image Id;
    # the legacy image store reports the config digest. Compare the recorded kind.
    if target["image_identity"] == "index_digest":
        observed_id = top_digest
    elif target["image_identity"] == "config_digest":
        observed_id = config_digest
    else:
        raise RuntimeError("Unknown image identity kind")
    require(
        observed_id == target["image_id"],
        "Registry tag contains another image; refuse overwrite or validation",
    )
    return {
        "exists": True,
        "repository": LOCK["repository"],
        "tag": target["tag"],
        "tag_digest": top_digest,
        "platform_digest": platform_digest,
        "image_id": target["image_id"],
        "image_identity": target["image_identity"],
        "config_digest": config_digest,
        "architecture": arch,
        "anonymous_read": True,
    }


def rows(path):
    with path.open() as stream:
        return list(csv.DictReader(stream))


def verify_evidence(arch, evidence):
    image = LOCK["images"][arch]
    # This workflow may add publication tooling, never silently change image inputs.
    subprocess.run(
        [
            "git",
            "diff",
            "--exit-code",
            LOCK["source_commit"],
            "--",
            ".docker",
            ".dockerignore",
            "extras",
            "gazebo",
            "models",
            "examples",
            "dave_interfaces",
        ],
        cwd=ROOT,
        check=True,
        stdout=subprocess.DEVNULL,
    )
    run, _ = request_json(
        f"https://api.github.com/repos/IOES-Lab/POSIM/actions/runs/{image['run_id']}",
        {
            "Authorization": "Bearer " + os.environ["GH_TOKEN"],
            "Accept": "application/vnd.github+json",
        },
    )
    require(
        run["head_sha"] == LOCK["source_commit"]
        and run["conclusion"] == "success"
        and run["run_attempt"] == image["attempt"],
        "Validated run provenance changed",
    )
    (evidence / "verified-run-metadata.json").write_text(
        json.dumps(
            {k: run[k] for k in ["id", "head_sha", "conclusion", "run_attempt", "html_url"]},
            indent=2,
        )
        + "\n"
    )
    require(
        (evidence / "tested-image-id.txt").read_text().strip() == image["image_id"],
        "Artifact image differs",
    )
    saved = read(evidence / "image-inspect.json")[0]
    actual = json.loads(docker("image", "inspect", image["image_id"]))[0]
    for info in [saved, actual]:
        require(
            info["Id"] == image["image_id"] and info["Architecture"] == arch,
            "Local/artifact image mismatch",
        )
        if image["image_identity"] == "index_digest":
            require(
                info.get("Descriptor", {}).get("digest") == image["image_id"],
                "Containerd index identity differs",
            )
        require(
            info["Config"]["Labels"]["org.opencontainers.image.revision"]
            == LOCK["image_revision"],
            "Image source label differs",
        )
    require(read(evidence / "inventory/result.json")["status"] == "PASS", "Inventory did not pass")
    receipt = read(evidence / "inventory/fuel-assets-receipt.json")
    require(
        receipt
        == {
            "status": "PASS",
            "lock_sha256": LOCK["fuel_lock_sha256"],
            "assets": 12,
            "files": 105,
            "nested_dependencies_present": True,
            "verification_downloads": 0,
        },
        "Fuel verification differs",
    )
    cases = {
        line.split("\t")[0]: line.split("\t")[1]
        for line in (ROOT / "extras/ci/quickstarts.tsv").read_text().splitlines()
        if line and not line.startswith("#")
    }
    repeat = [
        "spherical_world",
        "camera",
        "rexrov_waves",
        "ocean_current",
        "sea_pressure",
        "bluerov2",
        "bluerov2_heavy",
    ]
    expected = list(cases) + [f"{case}-r{n}" for n in range(2, 11) for case in repeat]
    summary = rows(evidence / "summary.csv")
    require(
        len(summary) == 78
        and {r["case"] for r in summary} == set(expected + ["inventory"])
        and all(r["status"] == "PASS" for r in summary),
        "Connected acceptance incomplete",
    )
    offline = rows(evidence / "offline-summary.csv")
    extra = [f"camera-offline-{n}" for n in range(1, 6)]
    require(
        len(offline) == 5
        and {r["case"] for r in offline} == set(extra)
        and all(r["status"] == "PASS" for r in offline),
        "Offline acceptance incomplete",
    )
    for record in expected + extra:
        directory = evidence / record
        result = read(directory / "result.json")
        case = "camera" if record in extra else re.sub(r"-r\d+$", "", record)
        require(result["command"] == shlex.split(cases[case]), f"Command mismatch: {record}")
        require(
            result["status"] == "PASS"
            and result["checks"]
            and all(v is True for v in result["checks"].values())
            and not result.get("error")
            and not result.get("faults"),
            f"Failed trial: {record}",
        )
        require(
            not fatal_log_lines((directory / "launch.log").read_text()),
            f"Raw child failure: {record}",
        )
        if record in extra:
            require(
                read(directory / "docker-network-mode.json") == "none"
                and (directory / "container-exit-code.txt").read_text().strip() == "0",
                "Offline evidence differs",
            )
    churn = read(evidence / "inventory/transport_churn/results.json")
    require(
        len(churn) == 5
        and all(
            r["pass"] and r["publisher_rc"] == r["subscriber_rc"] == 0 and not r["timeouts"]
            for r in churn
        ),
        "Transport regression incomplete",
    )
    return {
        "status": "PASS",
        "image_id": image["image_id"],
        "architecture": arch,
        "connected_trials": 77,
        "offline_trials": 5,
        "run_id": image["run_id"],
        "run_attempt": image["attempt"],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "mode", choices=["evidence", "registry-before", "registry-after", "pulled"]
    )
    parser.add_argument("architecture", choices=["arm64", "amd64"])
    parser.add_argument("--evidence", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.mode == "evidence":
        result = verify_evidence(args.architecture, args.evidence)
    else:
        result = registry_image(args.architecture, args.mode == "registry-before")
        if args.mode == "pulled":
            info = json.loads(
                docker("image", "inspect", LOCK["repository"] + "@" + result["tag_digest"])
            )[0]
            require(
                info["Id"] == result["image_id"] and info["Architecture"] == args.architecture,
                "Pulled image differs from tested image",
            )
            result["pulled_image_matches"] = True
            result["layer_cache_note"] = (
                "docker pull resolves the registry digest; pre-existing local layers may be reused."
            )
    args.output.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
