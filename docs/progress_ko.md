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
| 4 | 이 행을 포함한 커밋 | CTest 3/3; SHAKE 빈입력 KAT·15개 Python/OpenSSL 대조; 실제 EVP 초기화/비XOF 종료 실패 경로 확인 |
