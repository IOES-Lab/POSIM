export const sections = [
  { en: 'Getting started', ko: '시작하기', pages: [
    ['index', 'Overview', '소개', ['requirements', 'installation']],
    ['requirements', 'System requirements', '시스템 요구 사항', ['requirements']],
    ['install', 'Install on Ubuntu', 'Ubuntu 설치', ['native', 'installation']],
    ['docker', 'Docker environment', 'Docker 환경', ['docker']],
    ['quickstart', 'First simulation', '첫 시뮬레이션', ['quickstart']],
  ] },
  { en: 'Examples', ko: '예제', pages: [
    ['rovs', 'ROVs & BlueROV2', 'ROV와 BlueROV2', ['rovs']],
    ['surface', 'Surface robots & waves', '수상 로봇과 파도', []],
    ['gliders', 'Slocum glider', 'Slocum 글라이더', ['gliders']],
    ['worlds', 'World library', '월드 목록', ['worlds']],
    ['objects', 'Objects & task scenes', '물체와 작업 장면', ['objects']],
  ] },
  { en: 'Advanced guides', ko: '상세 가이드', pages: [
    ['custom-robots', 'Add a robot', '로봇 추가', ['add-robot']],
    ['heightmaps', 'Heightmap terrain', '높이맵 지형', ['heightmaps']],
    ['ros', 'ROS 2 & control', 'ROS 2와 제어', ['quickstart', 'rovs']],
    ['sonar-tuning', 'Sonar build & performance', '소나 빌드와 성능', ['sonar-build', 'sonar-optimization']],
    ['troubleshooting', 'Troubleshooting', '문제 해결', ['quickstart', 'sonar-build']],
  ] },
  { en: 'Plugin reference', ko: '플러그인 참고', pages: [
    ['currents', 'Ocean currents', '해류', ['currents']],
    ['camera', 'Underwater camera', '수중 카메라', ['camera']],
    ['pressure', 'Sea pressure', '수압', ['pressure']],
    ['dvl', 'Doppler velocity log', 'DVL', ['dvl']],
    ['usbl', 'USBL positioning', 'USBL 위치 추정', ['usbl']],
    ['coordinates', 'Spherical coordinates', '위도·경도 좌표', ['coordinates']],
    ['sonar', 'Multibeam sonar', '멀티빔 소나', ['sonar']],
  ] },
  { en: 'Project', ko: '프로젝트', pages: [
    ['libraries', 'Terrain, routing & control', '지형·경로·차량 제어', []],
    ['contributing', 'Contributing', '기여하기', ['development']],
    ['citation', 'Citation & licenses', '인용과 라이선스', ['development']],
  ] },
];
export const pages = sections.flatMap(section => section.pages.map(([slug, en, ko, sources]) =>
  ({ slug, en, ko, sources, sectionEn: section.en, sectionKo: section.ko })));
