# 최종 진행 상태와 재현 근거

계획1–36 구현·검증·인계 기준선 완료. 24번 이식성 수정1개와31번 연구 도구
이식성 수정2개를 별도 fix 커밋으로 기록했다. main은 새 루트부터39개 실제
커밋으로 구성되며 계획 번호와 총 Git 커밋 수는 다르다. 추가 브랜치는 만들지 않는다.
초기 기존16개 main 이력은 사용자 승인으로 새 이력에서 제외했다.

## 검증 범위

- 기능: 고정 순열/AEAD 벡터, 순열13472개 대조, AEAD400사례,1168비트 변조,
  인자799개,할당실패·실제 AD/메시지 각2^24B·guard·비정렬·동시4스레드800왕복.
- 전체 옵션의 CTest29개. Linux GCC14.2/OpenSSL3.5.7/Python3.12.14에서 Release와
  ASan/UBSan 각29/29 통과. 지역 LSan은 ptrace 제한으로 제외했고 CI의 default LSan과 구분한다.
- 원격 Linux GCC/Clang·Windows MSVC·macOS AppleClang·Linux sanitizer의5개 job을
  실행했다. 24번의 C4996/runner image 문제와31번의 OpenSSL include·Windows binary
  stdin 문제를 실제 실패 로그에서 찾아 수정했다. 수정 후5개 job 통과 기록은
  873f7e6 및9c86cc6의 Actions에 있다. 35번310a05c의5개 CI job에서 benchmark compile/smoke와 전체 회귀가 모두 성공했다.
  최종36번 commit도 동일 workflow의 실제 결과로 확인한다.
- 코드 검토:4개 public API 계약 대조, 생산 전역 mutable state 부재, GCC x86_64의
  소스/생성 코드에서 공개 길이/라운드 분기와 인증 후 memcpy 확인. 다른 CPU/컴파일러의
  전체 상수시간 증명, OpenSSL 내부 기계어/물리 부채널 분석은 미완료다.
- 유한 연구: 국소96위치×256쌍·동시MSB16384쌍,고정40검정,discovery330실험,
  holdout12검정,프레이밍10404쌍,AD누락/nonce중복 대조 완료. 목표 실행량·완료 수·
  생성 입력·core SHA·보정 검정군을 JSON에 남겼다. 전체 trail/hull·모드 보안 증명 등은 미탐색이다.
- 성능: 동일 Release 호스트5표본, P12/P16 및18작업×3알고리즘 전체 암복호화/인증실패,
  임시 payload 상한/객체 크기,키·nonce·가속·분산 차이를 기록했다. 다른 CPU/OS 성능은 미측정이다.
- 34번 LE64 후보는 정확성/연구/sanitizer를 통과했으나 대표 작업이 느려서 규범 대체 절차로
  생산 bytewise 코어를 유지했다. 실제 제목과 재현 도구/소스 지문은 최적화 문서에 있다.

## 재현과 Git 상태 확인

README의 clean configure/build, CTest, check-differential, research-smoke/full,
bench 및 sanitizer 명령을 Git 없는 새 소스 디렉터리에서 실행해 확인했다.
새 디렉터리 Release29/29·ASan/UBSan29/29, 독립대조·smoke/full·56행 benchmark가
모두 완료됐고 Git identity null에서도 core fingerprint와 결과가 일치했다.
예산0/키보드 중단은 이전 complete 파일을 incomplete로 교체하며, 각 연구의
완료 수와 중간 checkpoint를 보존하도록 확인했다. Git 없는 소스
복사본에서는 Git identity가 null이며 실제 C source fingerprint는 계속 기록한다.
최종 로컬/푸시 SHA는 자기참조 문자열을 문서에 고정하지 않고 아래 명령으로 확인한다.

```sh
git rev-parse HEAD
git rev-parse origin/main
git rev-list --count main
git log --reverse --format='%H %s'
```

최종 commit의 실제 원격 CI 상태는
[Actions](https://github.com/Koreapanda4444/CrossWeave-P384/actions/workflows/ci.yml)의
동일 SHA에서 확인한다. 과거 실패·중간 취소를 성공으로 바꾸지 않는다.
36번 완료는 구현·유한 연구·문서 인계 완료이며 암호학적 안전성 승인/상용 인증이 아니다.
후속 작업은 보안 문서의 미탐색과 외부 검토 질문이며 이번 계획의 미구현 기능은 없다.

## 계획과 실제 커밋

각 행은 해당 변경의 로컬 검증과 원격 커밋을 연결한다. 마지막 행은 그 행이
포함된 commit 제목으로 식별하며 나머지 SHA는 다음 변경에서 확정했다.

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
| 28 | `d1309cc2ee820e689b41383004deb6badd7be17b` | 55개 차분·384bit 집계·discovery/holdout 분리·동률규칙·30분상한·중단JSON 구현; C/Python counts·정확이항/Wilson/logp 경계·예산0 재현 검증 |
| 29 | `f0b585096fecb8681af14791baf3f85b0d67cdfa` | 입력bit383→출력bit1 고정가설: rounds2/4/8/12/16×8dataset×16384쌍40검정 완료; Wilson99·정확양측이항·Bonferroni40·underflow logp 기록; 과거11회 강제없음 |
| 30 | `bb9fb443668e5db9a1431443bc31d232565c496f` | AD/M길이0..33 및 독립3×3패턴10404쌍 실제 흡수전/후 state·domain·빈tail 전부대조; 프레임역해독/중복검사·정상코어 nonce재사용 첫블록XOR 대조 |
| 31 | `e0938149d0b8514e33ad69b4f42caadbf54b586c` | research-smoke/full complete(전체예산1800초 중13.30초); 국소96위치·고정40·discovery330·holdout12·프레이밍10404 전량완료; 소스/실행파일지문·가정/범위/미탐색·선정JSON 기록; testingOFF 연구build 확인 |
| 31-fix | `c0892348c650200dbd961a98692cdcc4e7fbc8e5` | 연구 target에서 OpenSSL 헤더를 직접 사용하는 omitted_ad에 명시 OpenSSL::Crypto 연결; Linux 기본 include가 숨긴 macOS/Windows CI 실패 수정 |
| 31-fix2 | `9c86cc61192ec2ae4da9acdb079d8b958f20affc` | Windows CRT text stdin의 CRLF/0x1a 변환으로 raw 연구 stream 단축 발견; 두 binary worker에만 O_BINARY 적용·0..255 byte 회귀 추가 |
| 32 | `c96ef7aecdfc4bf63f188cac53d2fc774afc3fce` | 단조시계·API포함지연·P12/P16·AD0/32/1024×메시지6크기; warmup후5표본·checksum·freshkey/카운터nonce·인증실패시간/출력불변·환경/메모리/객체크기 JSON 실측 |
| 33 | `95452ef18e204302072b5024333d03574824d226` | OpenSSL AES128GCM/ChaCha20Poly1305 전체 AEAD wrapper·KAT/왕복/변조불변/EVP정수상한 통과; 동일18작업×3알고리즘+P12/P16의5표본56행 실측; key/nonce·가속/EVP context 비용 구분 |
| 34 | `f9ae8fd71a30a445eaa911017036228e689b08f9` | LE64후보 Release29/29·ASan/UBSan29/29·full연구 통과했으나 AD32 대표3작업0.94~0.98배로 느림; 명세 대체절차로 bytewise코어 유지·복원후sanitizer29/29·정확후보지문 재현 도구/JSON 기록 |
| 35 | `310a05c54947cff74e147f1085baaea161afadd5` | 동일호스트 최종56행×5표본·중앙값/minmax·CPU/OS/compiler/OpenSSL/가속한계·전체API비용·GNUsize/heap상한 문서화; 과거참고값 분리·최적화거부 원표본 보존; 실제warmup 수 메타데이터 명시·CI benchmark compile/smoke 추가 |
| 36 | 이 행을 포함한 커밋 | 새 Git없는 소스디렉터리에서 configure/build·Release29/29·ASan/UBSan29/29·독립대조·smoke/full·56행bench 재현; Git없는지문·예산0/중단incomplete체크포인트 회귀; README/API/연구/성능/진행표 최종정리; 35번CI5job성공·최종SHA CI 별도확인 |
