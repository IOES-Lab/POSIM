// High-confidence regressions; editorial review still checks meaning and length.
const rules = [
  [/스탠드(?:얼|어)론/u, 'Use 설치형 앱.'],
  [/ArduPilot 임무를 제출하는 동작/u, 'Describe the control action directly.'],
  [/(?:기존|원래)\s*(?:덮개|캐노피)\s*색/u, 'Name the current colour.'],
  [/original canopy colo[u]?r/i, 'Name the current colour.'],
  [/문서 검토 시점|검토 시점\s*[（(]|at this review/i, 'Keep review dates in engineering records.'],
  [/별도 후보 버전|a separate candidate|historical test candidates|과거 검증용/i, 'Document supported behaviour; keep candidate history in review records.'],
  [/(?:이전|예전|과거)\s*(?:버전|검증|영상|TIFF|이미지)|(?:previous|earlier|historical)\s+(?:versions?|validation|TIFF|images?|captures?)/i, 'Describe the current product without version history.'],
  [/추가했습니다|적용했습니다|갱신했습니다|이 문서는.+정리한 것입니다/u, 'Describe present behaviour instead of implementation history.'],
];

export function checkPublicWording(markdown, file) {
  const prose = markdown.replace(/```[\s\S]*?```/g, '').replace(/`[^`]*`/g, '');
  for (const [pattern, advice] of rules) {
    const match = pattern.exec(prose);
    if (match) throw new Error(`${file}: public wording "${match[0]}". ${advice} See AGENTS.md.`);
  }
}
