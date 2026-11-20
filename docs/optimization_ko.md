# 완전 블록 LE64 경로의 측정과 결정

계획34번에서 AD와 암호문 피드백의16B 완전 블록을 두 LE64 word로 처리하는
후보를 구현했다. tail·도메인·라운드·태그·API 계약은 동일하게 유지했다.
후보는 Release29/29, ASan/UBSan29/29와 research-full 전체 회귀를 통과했다.
지역 환경의 LeakSanitizer 제한은 별도이며 CI의 누수 탐지 결과와 구분한다.

동일 호스트·Release·5표본에서 AD32B의 암호화 중앙값은 다음과 같다.

| 메시지 | 기존 ns | LE64 후보 ns | 기존/후보 속도비 |
| ---: | ---: | ---: | ---: |
| 16B | 2670.406 | 2724.695 | 0.9801 |
| 16KiB | 326916.906 | 347661.580 | 0.9403 |
| 1MiB | 21584384.750 | 22255286.750 | 0.9699 |

이번 호스트에서는 이득을 입증하지 못했고 세 대표 작업 모두 중앙값이 느려졌다.
따라서 규범 명세의 대체 절차에 따라 생산 코어는 기존 bytewise 경로를 유지한다.
실제34번 제목은 `docs: record full-block optimization results`다.
미리 정한 속도 수치에 맞추려고 라운드나 태그를 줄이지 않았다.

판정은 공유 가상화 호스트의 관측이며 다른 컴파일러·CPU에서의 불리함을
증명하지 않는다. 범위/중앙값/전체 표본은 bench/results의 JSON과 benchmark_ko.md를
참고한다. 후보의 소스 지문과 시험 범위는 bench/results/optimization.json에 있다.
생산 source는 복원 후 sanitizer29/29를 다시 통과했다.

버린 후보는 생산 경로에 남기지 않는다. 재현 도구는 build/ 아래에만 복사본을
생성하고, 두 함수의 동일 LE64 변형을 적용해 build/test/bench를 실행한다.
생성한 candidate의 전체 core_source_sha256이 실제 측정한 후보와 일치함을 확인했다.

```sh
python3 bench/evaluate_full_blocks.py --directory build/full-block-trial
```
