export const sections = [
  { en: 'Introducing POSIM', ko: 'POSIM 소개', pages: [
    ['index', 'Overview', '소개', ['requirements', 'installation']],
    ['features', 'Features & examples', '기능과 예제', []],
    ['libraries', 'Components & libraries', '구성 요소와 라이브러리', []],
  ] },
  { en: 'Getting started', ko: '시작하기', pages: [
    ['requirements', 'System requirements', '시스템 요구 사항', ['requirements']],
    ['install', 'Install on Ubuntu', 'Ubuntu 설치', ['native', 'installation']],
    ['install-macos', 'Install on macOS', 'macOS 설치', []],
    ['install-windows', 'Install on Windows', 'Windows 설치', []],
    ['docker', 'Docker environment', 'Docker 환경', ['docker']],
  ] },
  { en: 'Beginner guides', ko: '처음 배우는 사용법', pages: [
    ['quickstart', 'First simulation', '첫 시뮬레이션', ['quickstart']],
    ['visualization', 'View & record sensor data', '센서 데이터 보기와 기록', ['quickstart']],
    ['worlds', 'World library', '월드 목록', ['worlds']],
    ['rovs', 'ROVs & BlueROV2', 'ROV와 BlueROV2', ['rovs']],
    ['gliders', 'Slocum glider', 'Slocum 글라이더', ['gliders']],
    ['objects', 'Objects & task scenes', '물체와 작업 장면', ['objects']],
  ] },
  { en: 'Control robots', ko: '로봇 제어', pages: [
    ['ros', 'ROS terminals & topics', 'ROS 터미널과 토픽', ['quickstart', 'rovs']],
    ['python-control', 'Control with Python', 'Python으로 제어하기', ['rovs']],
    ['robot-control', 'Thrusters & autopilot', '추진기와 Autopilot', ['rovs']],
    ['surface', 'Surface robots & waves', '수상 로봇과 파도', []],
  ] },
  { en: 'Advanced guides', ko: '상세 가이드', pages: [
    ['custom-worlds', 'Create a world', '월드 만들기', ['worlds', 'heightmaps']],
    ['custom-robots', 'Add a robot', '로봇 추가', ['add-robot']],
    ['custom-sensors', 'Add & configure sensors', '센서 추가와 설정', ['camera', 'dvl']],
    ['heightmaps', 'Heightmap terrain', '높이맵 지형', ['heightmaps']],
    ['sonar-tuning', 'Configure & tune sonar', '소나 설정과 실험', ['sonar-build', 'sonar-optimization']],
  ] },
  { en: 'Plugin reference', ko: '플러그인 참고', pages: [
    ['currents', 'Ocean currents', '해류', ['currents']],
    ['underwater-rendering', 'Underwater scene effects', '수중 장면 효과', []],
    ['camera', 'Underwater camera', '수중 카메라', ['camera']],
    ['gnss', 'Marine GNSS', '해양 GNSS', []],
    ['pressure', 'Sea pressure', '수압', ['pressure']],
    ['dvl', 'Doppler velocity log', 'DVL', ['dvl']],
    ['usbl', 'USBL positioning', 'USBL 위치 추정', ['usbl']],
    ['coordinates', 'Spherical coordinates', '위도·경도 좌표', ['coordinates']],
    ['sonar', 'Multibeam sonar', '멀티빔 소나', ['sonar']],
  ] },
  { en: 'Reference & project', ko: '참고와 프로젝트', pages: [
    ['troubleshooting', 'Troubleshooting', '문제 해결', ['quickstart', 'sonar-build']],
    ['contributing', 'Contributing', '기여하기', []],
    ['citation', 'Citation & licenses', '인용과 라이선스', []],
  ] },
];
export const pages = sections.flatMap(section => section.pages.map(([slug, en, ko, sources]) =>
  ({ slug, en, ko, sources, sectionEn: section.en, sectionKo: section.ko })));
