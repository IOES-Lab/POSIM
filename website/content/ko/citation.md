# 인용과 라이선스

모델·환경·센서 사용 설명은 [기존 ROS 2 프로젝트 문서](https://dave-ros2.notion.site/)도 참고합니다. 실행 명령은 POSIM 패키지 이름과 이 문서의 ROS·Gazebo 환경을 기준으로 합니다.

POSIM은 IOES-Lab에서 관리합니다. 사용·배포 시 저자 표시와 구성 요소별 고지를 유지하세요.

## 기반 시뮬레이터 인용

DAVE 프로젝트의 해양 가상 환경은 다음 논문에서 설명합니다.

Zhang 외, *DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*, IEEE/OES Autonomous Underwater Vehicles Symposium, 2022. [DOI: 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808).

멀티빔 소나 모델은 다음 논문을 참고합니다.

Choi 외, *Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*, Frontiers in Robotics and AI, 2021. [DOI: 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646).

실험의 재현 기록에 POSIM 커밋, 로봇·월드 설정과 관련 의존성을 포함하세요. 전체 저자와 정확한 서지 정보는 원문 출판 기록을 사용합니다.

## 구성 요소의 출처

| 구성 요소 | 출처·고지 |
| --- | --- |
| POSIM의 Apache 소스 | 저장소 [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE), 파일별 고지 |
| DAVE 기반 | [Project DAVE](https://github.com/Field-Robotics-Lab/dave), 저자·소스 고지 |
| 외부 Wave Sim | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim), 업스트림 GPLv3 라이선스 |
| 외부 WAM-V·VRX 자산 | 내려받은 의존성에 보관된 고지 |
| Gazebo Fuel 모델 | 개별 자산의 출처와 라이선스 |

파도 빌드는 업스트림 소스와 수정 사항을 별도로 보관합니다. 의존성과 배포 고지는 [extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)를 참고하세요. 저장소가 분리되어 있어도 구성 요소의 라이선스 조건은 유지됩니다.

## 문서 출처

참고 자료는 [IOES-Lab POSIM Notion Wiki](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74)와 각 페이지 하단의 소스 링크에서 확인하세요.

## 관리 및 문의

한국해양대학교(KMOU)의 [IOES Lab.](https://lab.wschoi.com)에서 POSIM을 관리합니다.

연구실 문의는 [wschoi@kmou.ac.kr](mailto:wschoi@kmou.ac.kr)로 보내세요. 소프트웨어 문제는 [GitHub Issues](https://github.com/IOES-Lab/POSIM/issues)에 등록하세요.
