# 현재 코어의 전체 API 성능

측정 코어 Git SHA: `95452ef18e204302072b5024333d03574824d226`  
C 코어 source SHA256: `e0fd759a7c534df06ba42517f04be9a3c67bbaa5ff2e4eb177b17a94734e639d`

CPU: AMD EPYC 9V74 80-Core Processor  
OS: Linux-6.18.57-x86_64-with-glibc2.41  
컴파일러: cc (Debian 14.2.0-19) 14.2.0  
빌드: Release, -O3 -DNDEBUG, C11. 실제 전체 플래그와 OpenSSL 상세 버전·CPU 설정은
bench/results/final.json의 environment를 따른다. 같은 호스트에서 각 구간을 warmup한
뒤5개 측정 구간을 따로 수집했다. 표는 중앙값이고 JSON에 원 표본5개와 min/max를 남긴다.
시간은 플랫폼 단조시계 기준 ns, 아래 AEAD 표만 표시 편의를 위해 μs로 바꿨다.

벤치마크는 CMake의 bench target으로 재현한다. 각 프로세스에서 fresh random key를
만들고 모든 warmup/측정 암호화의 nonce 카운터를 증가시킨다. key/input/nonce 배열
생성은 측정 밖이다. API의 validation·임시 할당·초기화·AD·tag 및 소거·해제는
측정 안이다. checksum을 소비하고 유효 복호화 결과/인증실패 출력 불변을 확인한다.
벤치 프로세스마다 입력 객체는 새로 준비하므로 결과는 개별 API 호출 비용이다.

## 고정48B 순열 API

| 호출 | 중앙값 ns | min ns | max ns |
| --- | ---: | ---: | ---: |
| P12 | 313.449 | 311.469 | 337.178 |
| P16 | 417.993 | 402.878 | 461.833 |

## CrossWeave 전체 AEAD

| AD B | 메시지 B | 암호화 μs | 복호화 μs | 인증실패 μs | 암호화 MiB/s |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 0 | 1.560 | 1.493 | 1.591 | — |
| 0 | 16 | 1.749 | 1.718 | 1.752 | 8.725 |
| 0 | 64 | 2.816 | 2.857 | 2.691 | 21.676 |
| 0 | 1024 | 22.606 | 21.476 | 23.068 | 43.200 |
| 0 | 16384 | 319.096 | 316.870 | 321.813 | 48.967 |
| 0 | 1048576 | 21081.124 | 20380.701 | 20411.926 | 47.436 |
| 32 | 0 | 2.216 | 2.185 | 2.269 | — |
| 32 | 16 | 2.470 | 2.325 | 2.390 | 6.179 |
| 32 | 64 | 3.247 | 3.222 | 3.244 | 18.796 |
| 32 | 1024 | 22.225 | 21.853 | 21.922 | 43.941 |
| 32 | 16384 | 321.453 | 319.312 | 321.524 | 48.607 |
| 32 | 1048576 | 20673.671 | 20567.085 | 20638.462 | 48.371 |
| 1024 | 0 | 22.401 | 22.216 | 22.376 | — |
| 1024 | 16 | 21.423 | 21.312 | 21.915 | 0.712 |
| 1024 | 64 | 22.731 | 22.631 | 23.292 | 2.685 |
| 1024 | 1024 | 44.316 | 43.709 | 44.156 | 22.036 |
| 1024 | 16384 | 361.938 | 361.860 | 375.533 | 43.170 |
| 1024 | 1048576 | 21391.303 | 21665.619 | 21045.247 | 46.748 |

MiB/s의 분자는 메시지 길이이고 AD와 태그 처리는 시간에 포함한다. 빈 메시지의
처리량은 정의하지 않는다. 반복 수는 각 JSON 행의 iterations이고 작업량에 따라4~2048회다.
P12/P16은32회 warmup 후8192회씩 측정했다. 인증실패 시간 차이만으로 부채널 안전성을 입증하지 않는다.

## 동일 AD32B 비교

| 알고리즘 | key/nonce B | 메시지 B | 암호화 μs | 암호화 MiB/s |
| --- | --- | ---: | ---: | ---: |
| CrossWeave | 16/16 | 16 | 2.470 | 6.179 |
| CrossWeave | 16/16 | 16384 | 321.453 | 48.607 |
| CrossWeave | 16/16 | 1048576 | 20673.671 | 48.371 |
| AES-128-GCM | 16/12 | 16 | 0.620 | 24.608 |
| AES-128-GCM | 16/12 | 16384 | 3.769 | 4145.261 |
| AES-128-GCM | 16/12 | 1048576 | 182.355 | 5483.817 |
| ChaCha20-Poly1305 | 32/12 | 16 | 0.905 | 16.867 |
| ChaCha20-Poly1305 | 32/12 | 16384 | 6.551 | 2385.229 |
| ChaCha20-Poly1305 | 32/12 | 1048576 | 384.317 | 2602.022 |

전체54개 AEAD 작업과 순열2개의 표본은 final.json에 있다. OpenSSL AES128GCM과
ChaCha20Poly1305는 각 호출의 EVP context 생성·초기화·검증/태그·해제와 인증 전
평문 비공개용 임시 결과 비용을 포함한다. CrossWeave의 키16/nonce16과 ChaCha의
키32/nonce12, AES의 키16/nonce12 차이를 표에 명시했다. 공개 tag는 세 알고리즘 모두16B다.

OpenSSL은 기본 CPU feature dispatch를 사용하고 최적화 수준이 portable C와 다르다.
CPU feature 문자열은 기록했지만 실제 선택된 AES/SIMD 명령 경로는 별도 검증하지
않았다. 서로 다른 키 길이·성숙도·가속 수준을 통제한 수학적 우열 판정이 아니다.
관측상 이 호스트의 CrossWeave는 두 OpenSSL 기준 알고리즘보다 느리다.

## 메모리와 객체 크기

공식 API의 임시 heap은 메시지 nB이고 n=0이면 할당하지 않는다. 상한은16MiB다.
state/tag/순열 지역 배열과 컴파일러 stack 사용, malloc metadata는 별도다.
OpenSSL 비교 wrapper도 payload nB를 임시 보관하며 opaque EVP context의 추가
할당은 시간에는 포함하되 크기 상한은 측정하지 않았다. 프로세스 전체 RSS를
라이브러리 임시 메모리인 것처럼 보고하지 않는다.

GNU size Berkeley text열은 코드와 관련 read-only 데이터까지 포함한다.
archive 크기는 메타데이터도 포함하므로 두 값을 혼동하지 않는다.

```text
text	   data	    bss	    dec	    hex	filename
     69	      0	      0	     69	     45	crossweave_util.c.o (ex /workspace/scratch/adcc48dc4c60/CrossWeave-P384/build/libcrossweave.a)
    288	      0	      0	    288	    120	crossweave_constants.c.o (ex /workspace/scratch/adcc48dc4c60/CrossWeave-P384/build/libcrossweave.a)
   4905	      0	      0	   4905	   1329	crossweave_p384.c.o (ex /workspace/scratch/adcc48dc4c60/CrossWeave-P384/build/libcrossweave.a)
   9508	      0	      0	   9508	   2524	crossweave_aead.c.o (ex /workspace/scratch/adcc48dc4c60/CrossWeave-P384/build/libcrossweave.a)
    650	      0	      0	    650	    28a	crossweave_validate.c.o (ex /workspace/scratch/adcc48dc4c60/CrossWeave-P384/build/libcrossweave.a)
    106	      0	      0	    106	     6a	crossweave_memory.c.o (ex /workspace/scratch/adcc48dc4c60/CrossWeave-P384/build/libcrossweave.a)
```

정적 archive 크기: 25682B.

## 분산과 미해결 사항

공유 가상화 호스트의 클럭/스케줄링 분산이 있고5개 구간을 통계적으로 완전히
독립이라고 보장하지 않는다. 별도 전용 bare-metal 호스트·다른 CPU·다른 최적화
설정의 성능은 미측정이다. Windows/macOS CI의 기능 통과를 그 플랫폼의 벤치마크
결과로 쓰지 않는다. API 문서의 부채널/소거 한계와 보안 문서의 미탐색은 그대로다.

이전 감사의75.82MiB/s(16KiB·AD32B),9.86MiB/s(16B)는 다른 감사 코어·호스트의
참고값이고 이번 결과/합격선이 아니다. 측정 수치를 올리려고 라운드·태그를 줄이지 않았다.
LE64 후보의 전후 원 표본은 before.json/candidate.json, 판정은 optimization.json과
optimization_ko.md다. 이득을 입증하지 못해 생산 코어는 bytewise를 유지했다.
