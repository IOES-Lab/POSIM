"""Resolve the Ubuntu mirrors used by the CI jobs."""

import ipaddress
import json
import os
from pathlib import Path
import platform
import subprocess
from urllib.error import URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen


def system_address(host):
    try:
        result = subprocess.run(
            ["getent", "ahostsv4", host],
            capture_output=True,
            text=True,
            timeout=6,
        )
    except subprocess.TimeoutExpired:
        return None
    addresses = result.stdout.split()
    if result.returncode == 0 and addresses:
        return addresses[0]
    return None


def fallback_address(host):
    query = urlencode({"name": host, "type": "A"})
    for resolver in (
        "https://cloudflare-dns.com/dns-query",
        "https://dns.google/resolve",
    ):
        request = Request(
            f"{resolver}?{query}", headers={"Accept": "application/dns-json"}
        )
        try:
            with urlopen(request, timeout=8) as response:
                answers = json.load(response).get("Answer", [])
            for answer in answers:
                if answer["type"] == 1:
                    return str(ipaddress.IPv4Address(answer["data"]))
        except (URLError, TimeoutError, ValueError):
            continue
    raise SystemExit(f"::error::Could not resolve {host} on the CI runner")


def main():
    configure_hosts = os.environ["CONFIGURE_HOSTS"] == "true"
    if platform.machine() == "aarch64":
        mirrors = ["ports.ubuntu.com"]
    else:
        mirrors = ["archive.ubuntu.com", "security.ubuntu.com"]
        if configure_hosts:
            mirrors.append("azure.archive.ubuntu.com")

    mappings = []
    overrides = []
    for mirror in mirrors:
        address = system_address(mirror)
        if address is None:
            address = fallback_address(mirror)
            overrides.append(f"{address} {mirror}\n")
        mappings.append(f"{mirror}:{address}\n")
        print(f"{mirror} -> {address}", flush=True)

    if configure_hosts and overrides:
        entries = "".join(overrides)
        if os.geteuid() == 0:
            with Path("/etc/hosts").open("a") as hosts:
                hosts.write(entries)
        else:
            subprocess.run(
                ["sudo", "tee", "-a", "/etc/hosts"],
                input=entries,
                text=True,
                stdout=subprocess.DEVNULL,
                check=True,
            )

    with Path(os.environ["GITHUB_OUTPUT"]).open("a") as output:
        output.writelines(["hosts<<EOF\n", *mappings, "EOF\n"])


if __name__ == "__main__":
    main()
