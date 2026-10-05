# 기여하기

POSIM 소스와 문서에 작고 재현 가능한 변경을 제안할 수 있습니다. 모델, 제어기 설정과 문서의 내용을 함께 맞춰주세요.

## 브랜치에서 작업

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
git switch -c my-change
```

시뮬레이션 변경에는 맞는 [Ubuntu](install.md) 또는 [Docker](docker.md) 환경을 사용합니다. 생성된 빌드·설치 디렉터리와 개인 인증 정보는 커밋에 넣지 않습니다.

## 변경한 동작 확인

관련된 최소 예제를 선택합니다. 리소스 경로, 모델 배치, 시뮬레이션 시간, 데이터 수신과 정상 종료를 확인합니다. 제어 변경이면 제어기 동작도 확인하세요. 결과에는 소스 커밋, 렌더러, 아키텍처와 명령을 함께 기록합니다.

센서·동역학 변경에는 적절한 수치 비교가 필요합니다. 스크린샷은 외관을 보여주며 물리 모델 정확도를 증명하지 않습니다. 기존 구현을 수정할 때는 원저자의 출처 표시를 유지하세요.

## 문서 갱신

기본 언어는 영어(`/`), 한국어는 `/ko/`입니다. `website/content/en`과 `website/content/ko`의 대응하는 Markdown 파일을 함께 수정합니다. 페이지 순서는 공통이며 월드·물체 목록은 소스에서 생성합니다.

Node.js 24와 pnpm 10.11.0을 준비한 뒤 `website`에서 실행합니다.

```bash
pnpm install --frozen-lockfile
pnpm build
pnpm check
pnpm preview
```

`http://127.0.0.1:4174`를 열어 링크, 명령 복사, 검색, 언어 전환과 모바일 메뉴를 확인합니다. 호스팅과 유지보수 안내는 [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md)에 있습니다.

## Pull request 작성

문제, 변경 후 동작과 검증 내용을 설명합니다. 시뮬레이션 변경에는 명령·로그를, 화면 변경에는 데스크톱·모바일 화면을 포함합니다. 실행하지 않은 확인은 성공으로 표시하지 말고 리뷰 기록에 구분하여 남깁니다.

재현 가능한 오류는 [GitHub 이슈](https://github.com/IOES-Lab/POSIM/issues), 변경 검토는 [Pull request](https://github.com/IOES-Lab/POSIM/pulls)를 사용하세요. 프로젝트는 [한국해양대학교 IOES-Lab](https://lab.wschoi.com)에서 관리합니다.
