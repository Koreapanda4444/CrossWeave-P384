# 실행 기록

대상 명세: `docs/spec_ko.md` 개정 2.0. 이번 요청 범위: 계획 1–20.
소스·이력은 새로 작성한다. 초기화 전 기존 원격 main은
`d424070e56c9bd08e95dea3b379bf3da8adb17ec`였으며 다른 구현·이력이 있었다.
초기에는 기존 이력을 보존했고, 이후 사용자 추가 지시로 main 초기화를 승인받았다.
기존 16개 이력은 backup/pre-reset-20261118 브랜치에 보관했다.
부모 없는 새 1번 커밋부터 계획별 main 갱신을 순서대로 수행한다.
아래 SHA는 게시된 원격 커밋이며 로컬 검증 결과와 연결한다. 원격 CI는 24번 구현 전으로 미실행이다.

각 행은 해당 변경에 포함한 실제 로컬 검증이다. SHA는 다음 변경에서
확정하며 마지막 행은 그 행을 포함한 커밋의 제목으로 식별한다.
번호별 SHA는 `git log --reverse --format='%H %s'`로 재현할 수 있다.

| 계획 | 원격 SHA | 검증 |
| ---: | --- | --- |
| 1 | `c8eb5d82b167d1b065057f27614ac780d3cdfaae` | 명세 계획 1..36 연속·고정 대상·범위 확인; 빌드 미구현; 기존 원격 보존 |
| 2 | `c16aaf73ccb5b0c647d684417d0c41357fa2f975` | Release clean configure/build; CTest 환경 smoke 1/1; GCC 14.2·OpenSSL 3.5.7·8bit byte·uint64 확인 |
| 3 | `87cee07f693f015f56efb09c87ade55ce0a2ad01` | CTest 2/2; 비정렬 LE64·회전0/1/63/64·주소 overflow·경계 overlap·소거 메모리 확인 |
| 4 | `f1c390502540a3ce7f1a662df7143e9ecb50cd37` | CTest 3/3; SHAKE 빈입력 KAT·15개 Python/OpenSSL 대조; 실제 EVP 초기화/비XOF 종료 실패 경로 확인 |
| 5 | `60b7b82636c09cf94d7e86816fac905832c7beb4` | CTest 5/5; RC32개+IV 272B 전체 배열·생성기·Python/OpenSSL 일치; NUL·잘못된 IV 도메인 음성 대조 |
| 6 | `e1ba9ef06e7e20eab6c161b553a11a5824e03f88` | CTest 6/6; C와 독립된 수식 oracle; P12/P16 고정 KAT·32상태×17prefix 양방향 역변환 1088회 |
| 7 | `1c20ff18a33c981865e6c2cc4d4b96fcea55cbb0` | CTest 7/7; 216개 lane 중간값 C/Python 일치·overflow 입력·동시 갱신 음성 대조 |
| 8 | `b596494af96c801ecd7c3b8f6afc1a0805104885` | CTest 8/8; r0/11/15 중간 상태12개·P12/P16 8개 독립 대조·규범 zero KAT 일치 |
| 9 | `cc15ed4e2ba9c4df49577764d567e1f500e61024` | CTest 9/9; 8상태×17prefix C/Python 정·역변환과 양방향 복원; 역순열은 테스트 target에만 연결 |
| 10 | `944b07e9778b2cd5ce67fcbc2c9ae09211dacf57` | CTest 10/10; zero/ff/증가/80 상태8개 고정 KAT; C·독립 oracle 일치; 테스트 전후 vectors.json 불변 |
| 11 | `3be2a695119bfd8c419f7c65056f9bae772f956a` | CTest 11/11; 공개 P12/P16 KAT·정확한 in-place·양방향 부분 overlap·길이/capacity/NULL/주소overflow·실패/여유영역 불변 |
| 12 | `a3fc25cd335f41097267e402727d25cceaa754cf` | CTest 12/12; 1004상태 P12/P16+80상태×17prefix; C/Python 정·역변환 및 양방향 복원 총13472개 대조 |
| 13 | 이 행을 포함한 커밋 | 독립 Python AEAD: 명세 빈/17B KAT·태그 거부·정/역 trace 일치·0/16/32B 빈 최종 블록; 순열 oracle 재통과 |
