// Preserve section bookmarks when tutorials move to a more focused guide.
export const anchorRedirects = {
  features: {
    en: { 'public-feature-catalogue': 'features.html' },
    ko: { '공개-기능-목록': 'features.html' },
  },
  libraries: {
    en: { 'terrain-routing-and-control-libraries': 'libraries.html' },
    ko: { '지형-경로-차량-제어-라이브러리': 'libraries.html' },
  },
  ros: {
    en: {
      'ros-2-and-control': 'ros.html',
      'rexrov-thruster-commands': 'robot-control.html#rexrov-thruster-commands',
      'bluerov2-and-autopilot': 'robot-control.html#bluerov2-and-autopilot',
      'subscribe-from-python': 'python-control.html#subscribe-from-python',
      'visualization-and-recording': 'visualization.html#visualization-and-recording',
    },
    ko: {
      'ros-2와-제어': 'ros.html',
      'rexrov-추진기-명령': 'robot-control.html#rexrov-추진기-명령',
      'bluerov2와-autopilot': 'robot-control.html#bluerov2와-autopilot',
      'python에서-구독': 'python-control.html#python에서-구독',
      '시각화와-기록': 'visualization.html#시각화와-기록',
    },
  },
  quickstart: {
    en: {
      '1-start-an-ocean-world': 'worlds.html#start-a-world',
      '2-spawn-rexrov': 'quickstart.html#1-start-the-robot-and-world',
      '3-receive-a-camera-image': 'visualization.html#receive-a-camera-image',
      'desktop-viewing': 'visualization.html#desktop-viewing',
      'check-the-results': 'quickstart.html#3-check-simulation-time',
      'next-experiments': 'quickstart.html#continue-learning',
      'choose-launch-arguments': 'worlds.html#choose-launch-arguments',
    },
    ko: {
      '1-해양-월드-실행': 'worlds.html',
      '2-rexrov-배치': 'quickstart.html#1-로봇과-월드-시작하기',
      '3-카메라-영상-받기': 'visualization.html#카메라-영상-받기',
      '데스크톱에서-보기': 'visualization.html#데스크톱에서-보기',
      '실행-결과-확인': 'quickstart.html#3-시뮬레이션-시간-확인하기',
      '다음-실험': 'quickstart.html#이어서-배우기',
      'launch-인수-선택하기': 'worlds.html#launch-인수-선택하기',
    },
  },
};
