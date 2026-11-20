# 실행 기록

대상 명세: `docs/spec_ko.md` 개정 2.0. 이번 요청 범위: 계획 1–20.
소스·이력은 새로 작성한다. 초기화 전 기존 원격 main은
`d424070e56c9bd08e95dea3b379bf3da8adb17ec`였으며 다른 구현·이력이 있었다.
초기에는 기존 이력을 보존했고, 이후 사용자 추가 지시로 main 초기화를 승인받았다.
기존 16개 커밋은 새 main의 조상 이력에서 제외했다.
부모 없는 새 1번 커밋부터 계획별 main 갱신을 순서대로 수행한다.
아래 SHA는 게시된 원격 커밋이며 로컬 검증 결과와 연결한다. 원격 CI는 24번 구현 전으로 미실행이다.

각 행은 해당 변경에 포함한 실제 로컬 검증이다. SHA는 다음 변경에서
확정하며 마지막 행은 그 행을 포함한 커밋의 제목으로 식별한다.
번호별 SHA는 `git log --reverse --format='%H %s'`로 재현할 수 있다.

## 20번 체크포인트 검증 범위

- Linux x86_64, GCC 14.2.0, OpenSSL 3.5.7, CMake 4.4.5, Python 3.12.14.
- Release CTest 20/20. ASan/UBSan Debug CTest 20/20 (`ASAN_OPTIONS=detect_leaks=0`, `UBSAN_OPTIONS=halt_on_error=1`).
- 기본 LeakSanitizer 실행은 `LeakSanitizer has encountered a fatal error` 및 ptrace 제한 안내로 실패했다. 누수 탐지기 통과로 보고하지 않는다.
- 대체 할당 계측은 성공/인증실패 경로의 할당=해제, 남은 임시 할당0, free 직전 버퍼0, 마지막 tag16/state48 소거를 확인한다. 범용 누수 탐지기를 대체했다는 의미는 아니다.
- 계측 target은 실제 AEAD·인자검사·순열 소스를 재사용하고 메모리/소거 모듈만 테스트 구현에 연결한다. 생산 정적 라이브러리에는 계측 제어·역순열이 없다.
- `BUILD_TESTING=OFF` 새 configure/build 및 공개 헤더 C++11 구문 검사가 통과했다.
- Windows/macOS/Clang, 최대 길이의 실제 암복호화, 동시 호출, 추가256개 AEAD corpus, 암호분석·성능·기계어 검토는 이번 완료 주장에 포함하지 않는다.
- 다음 계획은 21번. 사용자 지시대로 1–20번을 새 루트부터 main에 게시한다. 원격 CI는 24번 구현 전으로 미실행이다.

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
| 13 | `6d693d7011a9557fb1a45a250e7a080c7c363e94` | 독립 Python AEAD: 명세 빈/17B KAT·태그 거부·정/역 trace 일치·0/16/32B 빈 최종 블록; 순열 oracle 재통과 |
| 14 | `684b4eba82382d148f0d3e14b4429d7453253e58` | C 초기화·key XOR·AD 0/1/15/16/17/31/32/33B 모든 단계 state 독립 대조; NULL/0 AD 경로·oracle/KAT 회귀 |
| 15 | `18300f00894266659b8c86bd29351e54e532183e` | 64개 AD/메시지 경계쌍 ciphertext·tag·모든 message/final state 독립 대조; C block/whole 복호화 feedback 동등 |
| 16 | `20b0c843cea205bd81a5e8f3d221e4aadfee4ae7` | 공개 encrypt: 빈/17B KAT·비중첩/정확한 in-place·capacity/NULL/overlap 실패의 ciphertext/tag 불변·여유 capacity 보존 |
| 17 | `4d13d70a76a6b3abed8442c38033a3bb99828bdb` | 공개 decrypt: 빈/17B KAT·정확한 in-place·틀린 태그의 평문 미공개와 입력 ciphertext/여유영역 보존; CRYPTO_memcmp 16B 비교 |
| 18 | `bf48c588c9e9dfe3c4c2f40460bb5b0211053999` | 고정 AEAD 146개=경계144+규범2; C/Python 암복호화·in-place·여유 capacity·빈/정배수 tail trace 일치; fixture label/index·nonce 중복없음 |
| 19 | `8691e861fe975280cbde7a3dd12483b336272109` | API 계약799개: 길이/capacity/NULL/주소overflow 우선순위·양방향 partial overlap·모든 output/input alias·읽기 alias 허용·실제 구간 경계·실패시 arena 전체 불변 |
| 20 | `006148f85944d9a57be152fbad4f1c55fbeb2635` | 할당/인증 실패·0/1/16/17/65B·in-place 실패 원자성·malloc(0) 회피·임시/tag/state 소거·할당해제 균형; Release20/20·ASan/UBSan20/20(LSan 환경제약 별도 기록) |
| 21 | `3d6d848a43f997a70d09033fd738d94ff8d1f415` | 고정 fixture1168개 단일 비트 변조; 분리/in-place 출력 불변; tag길이0/15/17 거부 |
| 22 | `c57a92d0da41f243742527996891554cb27787e2` | check-differential: 순열13472대조+AEAD400사례(경계144/추가256); fixture label/index·nonce·길이 JSON 기록·실패 최초차이 진단 |
| 23 | `aa011e4f5d59d6c570b06a3fd5f5aae9a2242b78` | Release23/23·ASan/UBSan23/23(LSan 제외); 실제 AD/메시지 각각2^24B 왕복·guard·비정렬100회·상한초과 접근전거부 |
| 24 | `1219bae0642663899981f68599231e7dfcad495b` | CI YAML 검증; 실제 공식 action v4/v5 SHA와 Windows OpenSSL 경로 확인; 원격 4플랫폼+sanitizer 결과는 게시 후 확인 |
| 24-fix | `873f7e6ef89678287a7f768976847c33915a1704` | Windows C4996은 bounded sscanf 테스트 target에만 표준 CRT 허용; macos15·검증된 Node24 action v6 SHA로 갱신; 이전 CI 실패/취소 보존 |
| 25 | `9d8a4edb06d1d6ace5526a078ace17cfa86fbc21` | CI수정 후 LinuxGCC/Clang·WindowsMSVC·macOSAppleClang·LSan포함sanitizer 5job 성공; API소유권/alias/부채널 문서·GCC x86_64 기계어 검토·4스레드800왕복 |
| 26 | `ce277f795b0072219562b640cc2afd5d2e481b0d` | 의도적 tail XOR 단일누락: AD00→01 치환 수락과 정상코어거부; 1/15/17/31/33B 회귀·각 독립대조1질의·정상/결함 태그분리 |
| 27 | `1dfb414c1fa63833ea7f542c51e71737444fb524` | 96개(r,i) 각256쌍의 두 MSB항등식 C/Python대조; r0/i0 동시MSB16384쌍 지원집합·빈도확인; 독립가정/조건부확률 범위 기록 |
| 28 | 이 행을 포함한 커밋 | 55개 차분·384bit 집계·discovery/holdout 분리·동률규칙·30분상한·중단JSON 구현; C/Python counts·정확이항/Wilson/logp 경계·예산0 재현 검증 |
