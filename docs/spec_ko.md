# CrossWeave-P384 신규 개발 명세서와 커밋 계획

문서 개정: 2.0 / 작성일: 2026-11-09  
신규 대상: `Koreapanda4444/CrossWeave-P384` / 작업 브랜치: `main`  
범위: C11 순열·AEAD 라이브러리, 독립 검증, 제한된 암호분석, 성능 측정 / 계획: **36개 커밋**

## 1. 이 파일로 시작하는 방법

이 파일 하나를 새 Work에 첨부하면 된다. 별도의 지시문, 원본 ZIP, 증거 ZIP, 이전 채팅이나 다른 암호의 명세는 필수 입력이 아니다. 문서 개정 번호는 문서 버전이며 암호 알고리즘 이름에 붙이지 않는다.

새 작업 디렉터리와 새 Git 이력에서 아래 순서대로 구현한다. 예전 저장소의 소스 복사, clone, cherry-pick, 커밋 이력 승계는 하지 않는다. 기존 저장소는 수정·삭제하지 않는다. 새 원격의 이름은 위 대상과 일치하는지 확인하고, 존재 여부와 접근 권한은 실제 확인 결과로 기록한다. 새 원격에 README·LICENSE 같은 초기 파일만 있으면 보존한다. 예상하지 못한 구현이 이미 있으면 덮어쓰지 말고 확인한다.

사용자가 `N번까지`를 지정하면 그 커밋까지 구현·검증·커밋·가능한 푸시를 수행한다. 범위 지정이 없으면 1번부터 계획 순서대로 진행한다. 원격 미생성이나 푸시 권한 부족은 원격 작업의 장애이지, 가능한 로컬 구현·검증·커밋까지 중단할 이유는 아니다. 인증·승인 절차를 우회하지 않는다.

CrossWeave는 384비트 순열과 이를 사용하는 단일 AEAD 연구 후보다. P12와 P16은 별도 후보가 아니라 같은 순열의 고정된 12·16라운드 호출이다. 아래 사양에 맞는 구현을 완성하고, 재현 가능한 분석과 성능 자료를 만드는 것이 이번 작업이다. 실제 기밀 보호에 대한 적합성은 미확정이며, 테스트 통과를 보안 증명으로 쓰지 않는다.

이번 범위에서 라운드 수·상수 도메인·IV·흡수 순서·태그 길이를 임의 변경하지 않는다. 결함이나 강한 공격을 찾으면 최소 재현, 영향 범위와 수정 제안을 기록하고 사용자 승인 전에 수식을 바꾸지 않는다. 구현 결과에 맞추려고 기준 벡터를 바꾸지 않는다. 전체 라운드에 대한 보안 증명을 얻을 때까지 개발을 끝없이 반복하는 계획도 아니다.

GUI, 서버, DB, 파일 암호화 프로그램, 별도 운용모드, 언어 바인딩, SIMD·어셈블리·GPU 최적화는 이번 범위에 넣지 않는다.

## 2. 통일 기술 구성과 저장소 구조

### 2.1 기술 구성

- 공식 코어: C11, CMake 3.20 이상, CTest. 고정폭 부호 없는 정수와 명시적인 little-endian 로드·스토어를 사용한다.
- 외부 암호 의존성: OpenSSL 3.x `libcrypto`. SHAKE256 대조, `CRYPTO_memcmp`, `OPENSSL_cleanse`, 비교용 벤치마크에 사용한다. SHAKE256을 새로 구현하지 않는다.
- RC와 IV는 공개 상수다. Python 3.11 이상 표준 라이브러리 `hashlib.shake_256`으로 생성한 `const` 배열을 공식 C 코어에 넣고, 테스트에서 OpenSSL 및 Python으로 다시 계산해 대조한다. 메시지·키별로 RC/IV를 다시 생성하지 않는다.
- 독립 검증 코어: `tests/reference.py` 하나. 순열·AEAD 로직을 수식에서 따로 구현하며 C 코어를 호출해 기대값을 만들지 않는다. Python과 C의 SHAKE 백엔드가 동일할 수 있으므로 SHAKE 자체까지 독립 구현이라고 주장하지 않는다.
- 기본 산출물은 정적 라이브러리와 검증용 실행 파일이다. 공유 라이브러리 배포·설치 패키지는 이번 필수 범위가 아니다.

### 2.2 최상위 구조

| 경로 | 역할 |
| --- | --- |
| `include/crossweave.h` | 아래에 확정한 공개 API만 제공 |
| `src/` | 순열·AEAD·버퍼 검증·공개 상수·내부 헤더 |
| `tests/` | CTest, `vectors.json`, `reference.py`, `vector_runner.c`, 결정적 입력 생성·교차 검증 |
| `research/` | 축소 라운드 실험, 의도적 결함 대조군, 프레이밍·nonce 연구 |
| `research/results/` | 선정한 작은 JSON 재현 결과만 커밋 |
| `bench/` | 순열과 전체 AEAD 및 비교 알고리즘 측정 |
| `docs/spec_ko.md` | 이 문서의 규범 사양·개발 기준·커밋 계획을 담는 단일 원본 |
| `docs/api_ko.md` | 포인터·버퍼·오류·소유권 설명 |
| `docs/security_ko.md` | 공격 모델·결과·한계·미해결 위험 |
| `docs/benchmark_ko.md` | 측정 조건·표본·성능 결과 |
| `docs/progress_ko.md` | 계획 번호, 실제 SHA, 검증·푸시·CI 상태 |
| `CMakeLists.txt`, `README.md`, `.gitignore` | 빌드·실행·추적 제외 규칙 |
| `.github/workflows/ci.yml` | 실제 테스트를 실행하는 CI |

세 프로젝트의 최상위 구조와 문서 역할은 동일하다. 암호마다 필요한 내부 C 파일 개수는 달라도 된다. 실제 사용하는 파일만 생성하고 빈 폴더, 과거 코어, `legacy/`, 중첩 백업, 빌드 바이너리와 대량 로그는 커밋하지 않는다. 생성 결과와 임시 원시 자료는 `build/` 또는 `build-sanitize/` 아래에 두고 추적에서 제외한다.

공개 코어는 `crossweave_p384.c`, `crossweave_aead.c`로 나눈다. 내부 공통 기능은 `crossweave_internal.h` 및 필요한 작은 모듈에 둔다. RC/IV 생성 도구는 테스트 도구이며 빌드 때마다 네트워크에서 받아오지 않는다. 축소 라운드, 중간 상태 출력, 역순열, 결함 대조군은 설치 대상 공개 헤더에 넣지 않고 테스트·연구용 target에서만 접근한다.

## 3. 규범 순열 P384

### 3.1 상태와 연산

입출력은 48바이트다. 각 8바이트를 little-endian으로 해석해 `(s0,s1,s2,s3,s4,s5)`의 6개 64비트 lane으로 만든다. 직렬화도 같은 순서의 LE64다.

`ROL64(x,n)`은 64비트 순환좌회전, `ROR64`는 역회전이다. 덧셈은 `mod 2^64`, `^`는 XOR, `&`는 AND다. C의 signed overflow, 64 이상 shift, 정렬이 맞지 않는 포인터 캐스팅에 의존하지 않는다.

각 라운드 인덱스 `r=0..15`, lane 인덱스 `i=0..5`에 대해:

```text
alpha = ((7*i + 3*r) mod 63) + 1
beta  = ((13*i + 5*r) mod 63) + 1

f(a,b,r,i) = ROL64(a,alpha)
           ^ ROL64(b,beta)
           ^ (a & ROL64(b,11))
           ^ ((a+b) mod 2^64)
```

### 3.2 공개 상수

`SHAKE256(input,n)`은 입력 바이트열에 대한 SHAKE256 출력의 첫 `n`바이트다. 각 `r`에서 `j=0,3`의 두 상수를 다음과 같이 정한다.

```text
text = "cw-p384/round-" + decimal(r) + "/lane-" + decimal(j)
RC[r][j] = LE64(SHAKE256(ASCII(text), 8))
IV = SHAKE256(ASCII("CrossWeave/IV"), 16)
```

십진수 문자열에 선행 0, 공백, 개행을 넣지 않는다. 두 도메인 모두 뒤에 NUL을 붙이지 않는다. C 배열을 `RC[16][2]`로 저장하면 두 번째 첨자 0은 규범 `j=0`, 두 번째 첨자 1은 규범 `j=3`이다.

상수 생성 확인값:

- `RC[0][0]`의 64비트 정수값: `0xcab26712b1223857`.
- `RC[0][3]`의 64비트 정수값: `0x52cf70dd96cd3c07`.
- IV 바이트열: `8d65b4d7917d81bbd9803c03a1164043`.

### 3.3 순차 lane 갱신과 재배열

각 라운드는 다음을 순서대로 수행한다. XOR 갱신을 동시에 계산하면 다른 순열이 된다.

```text
for i in (0,2,4,1,3,5):
    s[i] = s[i] ^ f(s[(i+1)%6], s[(i+3)%6], r, i)

s[0] = s[0] ^ RC[r][0]
s[3] = s[3] ^ RC[r][3]

PERM = (2,5,1,4,0,3)
ROT  = (1,7,19,31,43,57)
new_s[i] = ROL64(s[PERM[i]], ROT[i]) for all i
s = new_s
```

마지막 재배열·회전은 마지막 라운드까지 매번 수행한다. `new_s` 계산은 갱신 전 `s` 전체를 사용한다.

- P12: `r=0..11`.
- P16: `r=0..15`.
- 연구용 `P_n`: `r=0..n-1`, `n=0..16`. `P_0`는 항등이다. 상수·라운드 인덱스를 다시 시작하거나 별도 도메인을 쓰지 않는다.

연구용 역라운드는 `old_s[PERM[i]]=ROR64(new_s[i],ROT[i])`로 재배열을 취소하고 상수 XOR를 제거한 뒤 `(5,3,1,4,2,0)` 순서로 같은 XOR 갱신을 취소한다. 여러 라운드를 되돌릴 때는 라운드 인덱스도 역순이다.

## 4. 규범 AEAD

### 4.1 입력과 초기화

| 항목 | 크기·규칙 |
| --- | --- |
| 비밀키 | 정확히 16바이트 |
| Nonce | 정확히 16바이트; 같은 키 아래 암호화 호출마다 고유 |
| 인증 태그 | 정확히 16바이트; 절단 태그 미지원 |
| state / rate / capacity | 48 / 16 / 32바이트 |
| AD, 평문 또는 암호문 | 각각 0 이상 `2^24`바이트 이하 |
| 암호문 길이 | 평문 길이와 동일; 태그는 별도 버퍼 |

```text
S = P16(nonce[16] || key[16] || IV[16])
S[16:32] ^= key[16]
```

이하 `S`는 48바이트 배열이다. 배열 구간은 시작 포함·끝 제외이며 XOR는 바이트별 연산이다. 순열 호출 시에는 3절의 LE64 규칙으로 변환한다. `S[47]`은 마지막 바이트다.

`2^24`는 초기 구현의 입력 상한이지, 키당 총 처리량이나 안전한 호출 수에 대한 보안 한계가 아니다. 고유 nonce 생성·수명 관리·중복 방지는 호출자 책임이다. 복호화 API는 같은 패킷을 다시 받았는지 검사하지 않으며 재전송 방지 기능을 제공하지 않는다.

### 4.2 AD 흡수

각 16바이트 완전 AD 블록마다:

```text
S[0:16] ^= ad_block
S[47] ^= 0x11
S = P12(S)
```

완전 블록을 모두 처리한 뒤 잔여 `tail` 길이를 `a`라 하면 `0 <= a < 16`이며:

```text
S[0:a] ^= tail[0:a]
S[a] ^= 0x80
S[47] ^= 0x12
S = P12(S)
```

AD가 비어 있거나 길이가 16의 배수여도 `a=0`인 마지막 단계를 반드시 한 번 수행한다. **잔여 바이트 XOR를 패딩 전에 수행한다.** 이를 빼는 선택지는 공식 코어에 없다.

### 4.3 메시지 처리

각 16바이트 완전 평문 블록마다:

```text
c = p ^ S[0:16]
S[0:16] = c
S[47] ^= 0x21
S = P12(S)
```

마지막 잔여 평문 길이를 `m`이라 하면 `0 <= m < 16`이며:

```text
c[0:m] = p[0:m] ^ S[0:m]
S[0:m] = c[0:m]
S[m] ^= 0x80
S[47] ^= 0x22
S = P12(S)
```

평문이 비어 있거나 길이가 16의 배수여도 `m=0`인 마지막 단계를 반드시 수행한다.

복호화에서는 `p=c XOR S[0:n]`을 계산한 후 state rate에 **입력 암호문**을 저장한다. 평문을 state에 저장하면 안 된다. 블록별 도메인·패딩·순열 호출은 암호화와 동일하다.

### 4.4 최종화와 인증

```text
S[16:32] ^= key[16]
S[47] ^= 0x31
S = P16(S)
tag[0:16] = S[0:16] ^ key[0:16]
```

복호화는 16바이트 태그 전체를 `CRYPTO_memcmp`로 비교한다. 검증 전에 호출자에게 평문을 쓰거나 콜백·로그로 내보내지 않는다. 내부 임시 결과에만 보관하고 인증 성공 후 한 번에 호출자 출력으로 복사한다. 실패하면 임시 평문과 state를 소거한 뒤 반환한다. 소거가 레지스터·캐시·외부 메모리 복사본까지 완전히 제거한다고 주장하지 않는다.

## 5. 확정 공개 API와 버퍼 계약

### 5.1 헤더

공개 헤더는 `<stddef.h>`, `<stdint.h>`를 포함한다. C++ 호출용 `extern "C"` 가드를 제공하되 C++ 구현을 추가하지 않는다.

```c
#define CROSSWEAVE_KEY_BYTES 16u
#define CROSSWEAVE_NONCE_BYTES 16u
#define CROSSWEAVE_TAG_BYTES 16u
#define CROSSWEAVE_RATE_BYTES 16u
#define CROSSWEAVE_STATE_BYTES 48u
#define CROSSWEAVE_MAX_INPUT_BYTES ((size_t)16777216)

typedef enum {
    CROSSWEAVE_OK = 0,
    CROSSWEAVE_EINVAL = 1,
    CROSSWEAVE_ELENGTH = 2,
    CROSSWEAVE_ECAPACITY = 3,
    CROSSWEAVE_EOVERLAP = 4,
    CROSSWEAVE_ENOMEM = 5,
    CROSSWEAVE_EAUTH = 7
} crossweave_status;

crossweave_status crossweave_p384_12(uint8_t *out, size_t out_capacity,
                     const uint8_t *in, size_t in_len);
crossweave_status crossweave_p384_16(uint8_t *out, size_t out_capacity,
                     const uint8_t *in, size_t in_len);

crossweave_status crossweave_encrypt(
    uint8_t *ciphertext, size_t ciphertext_capacity,
    uint8_t *tag, size_t tag_capacity,
    const uint8_t *plaintext, size_t plaintext_len,
    const uint8_t *ad, size_t ad_len,
    const uint8_t *nonce, size_t nonce_len,
    const uint8_t *key, size_t key_len);

crossweave_status crossweave_decrypt(
    uint8_t *plaintext, size_t plaintext_capacity,
    const uint8_t *ciphertext, size_t ciphertext_len,
    const uint8_t *tag, size_t tag_len,
    const uint8_t *ad, size_t ad_len,
    const uint8_t *nonce, size_t nonce_len,
    const uint8_t *key, size_t key_len);
```

AEAD는 one-shot API로 고정하며 영구 컨텍스트나 streaming API를 추가하지 않는다. P12/P16의 공개 함수는 고정된 완전 호출만 제공한다. 공개 API에는 임의 `rounds`, 중간 state callback, `omit_ad_tail` 같은 옵션이 없다.

### 5.2 인자·길이·오류

1. 정수 길이부터 검사한다. key/nonce 길이는 각각 16, 복호화 tag 길이는 16, 순열 입력 길이는 48이어야 한다. AD와 메시지 길이는 각각 상한 이하여야 한다. 위반은 `CROSSWEAVE_ELENGTH`다.
2. 필요한 출력 길이보다 capacity가 작으면 `CROSSWEAVE_ECAPACITY`다. 순열은 48바이트, 암호화 태그는 16바이트, 데이터 출력은 입력 데이터 길이만 필요하다.
3. key/nonce/tag/순열 입출력 포인터는 NULL일 수 없다. AD는 길이 0일 때 NULL을 허용한다. 메시지 입력은 길이 0일 때 NULL을 허용한다. 데이터 출력 NULL은 필요한 데이터 길이와 capacity가 모두 0일 때만 허용한다. 그 밖의 NULL은 `CROSSWEAVE_EINVAL`다.
4. 아래 overlap 정책을 검사한다. 지원 플랫폼의 `uintptr_t`로 주소 구간을 표현하고 주소+길이 overflow를 먼저 확인한다. 주소범위 표현 불가는 `CROSSWEAVE_EINVAL`, 금지된 겹침은 `CROSSWEAVE_EOVERLAP`이다. 서로 무관한 포인터를 C의 `<`나 포인터 뺄셈으로 비교하지 않는다.
5. 필요한 임시 메모리를 할당한다. 실패는 `CROSSWEAVE_ENOMEM`이며 출력은 그대로 둔다. 길이 0인 메시지는 `malloc(0)`의 구현별 동작에 의존하지 않는다.
6. 유효한 복호화 인자의 태그 불일치는 `CROSSWEAVE_EAUTH`다. 오류 우선순위는 이 순서로 고정하고 여러 오류가 동시에 있는 입력의 테스트도 그 순서를 따른다.

호출자는 길이·capacity에 맞는 살아 있는 객체를 제공해야 한다. 임의의 dangling pointer, 거짓 capacity, 해제된 메모리, 동시에 변경되는 입력의 유효성을 C 라이브러리가 알아낼 수 있다고 약속하지 않는다. 인자 검사로 거부되는 길이에 대해서는 해당 버퍼 내용을 읽지 않는다.

### 5.3 Alias와 출력 원자성

겹침은 capacity 전체가 아니라 **실제 읽거나 쓰는 길이의 구간**으로 판단한다. 길이 0인 구간은 겹침에 포함하지 않는다. 읽기 전용 입력끼리의 겹침은 허용한다.

| 호출 | 허용 | 거부 |
| --- | --- | --- |
| P12/P16 | `out == in`인 정확한 in-place, 또는 비중첩 | 서로 다른 시작 주소의 부분 중첩 |
| 암호화 데이터 | `ciphertext == plaintext`인 정확한 in-place, 또는 비중첩 | 부분 중첩; ciphertext 쓰기 구간과 key·nonce·AD의 겹침 |
| 암호화 태그 | 다른 모든 입력·출력 구간과 비중첩 | tag 16바이트와 plaintext·ciphertext·key·nonce·AD의 모든 겹침 |
| 복호화 데이터 | `plaintext == ciphertext`인 정확한 in-place, 또는 비중첩 | 부분 중첩; plaintext 쓰기 구간과 tag·key·nonce·AD의 겹침 |

예를 들어 데이터 끝 다음 주소에 별도 태그 16바이트를 두는 배치는 두 구간이 겹치지 않으면 허용한다. 데이터 입력이 AD와 겹친 상태에서 in-place 출력을 요청하면 출력과 AD의 겹침 규칙 때문에 거부한다.

모든 API에서 실패 시 호출자 데이터 출력은 바이트 단위로 불변이다. 암호화 실패 시 ciphertext와 tag **둘 다** 불변이어야 한다. 성공 시에도 실제 결과 길이 밖의 여유 capacity는 변경하지 않는다.

순열은 48바이트 지역 버퍼에서 계산한 뒤 출력한다. 암호화·복호화는 모든 입력 검사를 끝낸 후 메시지 길이의 임시 결과 버퍼와 지역 tag/state를 사용한다. 계산 중 실패가 없는 상태까지 끝낸 다음에만 출력에 복사한다. 복호화의 출력 복사는 태그 검증 이후다. 메시지 크기에 비례한 임시 할당 비용은 성능 측정에 포함한다. 키·nonce·AD 등 입력을 저장하거나 소유권을 가져가지 않는다.

## 6. 고정 기준 벡터와 독립 검증

아래 값은 명세 재현용으로 대조한 값이지 외부 인증 벡터가 아니다. 이미 존재하는 감사 결과를 새 저장소 구현의 통과 결과로 보고하지 않는다. 새 C/Python 구현이 이 값과 일치하는지 다시 실행해 확인한다.

### 6.1 SHAKE와 순열

SHAKE256의 빈 입력 64바이트 출력:

```text
46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762f
d75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be
```

위 두 줄은 개행 없이 연결한 128자리 hex다.

| 항목 | Hex |
| --- | --- |
| P12/P16 입력 | 48바이트 모두 `00` |
| P12 출력 | `f98e26352d066afa483b96a5fad30f0963895e163d529708102e9e8728393b8bb113373987dbf0066981e2fb0ffc2630` |
| P16 출력 | `59e853991a8c3b868074ee51104a750d54ca24a15315468d72233a7ed2126b5110b0ef086692e1cc7ba01d0648b15c8f` |

### 6.2 빈 AD·메시지

| 항목 | Hex |
| --- | --- |
| key | `f38f855b4280efb6c827a2361a03979e` |
| nonce | `00000000000000000000000000000000` |
| AD, plaintext, ciphertext | 모두 빈 바이트열 |
| tag | `7c7d20f684fe8f6d80ae7d20c78cc3cf` |

### 6.3 완전 블록과 잔여 블록을 함께 지나는 벡터

| 항목 | Hex |
| --- | --- |
| key | `00000000000000000000000000000000` |
| nonce | `000102030405060708090a0b0c0d0e0f` |
| AD | `000102030405060708090a0b0c0d0e0f10` |
| plaintext | `000102030405060708090a0b0c0d0e0f10` |
| ciphertext | `d9284e96da9b49763c268a2abc39abf2c0` |
| tag | `c1157d9abecbade2ca62f2611adc0c4a` |

### 6.4 테스트 구성

- `tests/vectors.json`은 고정 기대값을 추적한다. 테스트 실행 때 C 코어 출력으로 기대값을 덮어쓰지 않는다.
- 독립 oracle은 임의 48바이트 상태의 prefix 0..16, 역변환, AEAD를 제공한다. 테스트 전용 API에서 C 중간값과 대조한다.
- 순열 대조: 고정 4개 상태(`00`, `ff`, 증가 바이트, 반복 `80`)와 결정적 1,000개 상태에서 P12/P16을 대조하고 양방향 역변환을 확인한다. 별도 80개 상태에서 17개 prefix를 대조한다.
- AEAD 대조: AD/메시지 길이 집합 `{0,1,15,16,17,31,32,33,255,256,257,1024}`의 144개 조합과 결정적 256개 추가 사례를 확인한다. 실제 최대 길이 `2^24`의 유효 입력도 별도 C 테스트로 처리한다. 대형 입력을 Python oracle로 전부 처리하는 것을 기본 CI 조건으로 삼지 않는다.
- 테스트 입력 생성은 `SHAKE256(ASCII("CrossWeave/tests/") || ASCII(label) || 0x00 || LE64(index), n)`의 앞 `n`바이트로 고정한다. label은 `permutation`, `prefix`, `key`, `ad`, `message`, `lengths`로 분리한다. AEAD nonce는 사례 index의 LE128이며 같은 fixture 생성에서 중복하지 않는다.
- 추가 길이 사례의 `lengths` 출력 첫 두 LE32 값을 각각 `mod 513`, `mod 1025`로 매핑한다. 키·AD·메시지의 label/index는 결과 JSON에 남긴다.
- 변조 fixture는 key `00..0f`, nonce `10..1f`, AD 길이 33의 증가 바이트, 메시지 길이 65의 증가 바이트다. key·nonce·AD·ciphertext·tag의 각 비트를 단독 반전하고 인증 실패 및 출력 불변을 확인한다. 이 한 fixture의 통과를 모든 위조에 대한 증명으로 쓰지 않는다.
- 부정 테스트는 짧거나 긴 key/nonce/tag, 상한 초과, 작은 출력 용량, NULL, 정확한 in-place, 양방향 부분 overlap, 출력 간·비밀 입력과의 alias, 할당 실패를 포함한다. 테스트용 내부 할당 실패 주입을 사용하며 이를 공개 사용자 hook으로 노출하지 않는다.

`check-differential`은 여기서 **독립 C/Python 구현 결과 대조**를 뜻한다. 7절의 차분 암호분석과 다른 작업이다.

## 7. 유한한 연구 계획과 판정 규칙

### 7.1 반드시 재현하는 정확한 성질

**잔여 AD 누락 대조군.** 공식 코어가 아닌 연구 전용 최소 변형에서 4.2절의 `S[0:a] ^= tail`만 생략한다. 키와 nonce를 고정하고 `AD=00`, 평문은 ASCII `one valid encryption query`로 한 번 암호화한다. AD만 `01`로 바꾼 동일 ciphertext/tag가 그 대조군에서는 수락되는지 확인한다. 공식 코어는 같은 형식의 변조를 거부해야 한다. 정상 코어의 태그와 결함 대조군의 태그를 혼용하지 않는다. AD 길이 1,15,17,31,33의 마지막 바이트 변경 회귀도 고정한다. 이 결과는 의도적으로 생략한 모델에 대한 것이며 보지 않은 외부 저장소의 취약점 판정이 아니다.

**국소 함수의 MSB 관계.** `M=2^63`에서:

```text
f(a^M,b,r,i) ^ f(a,b,r,i)
  = ROL64(M,alpha) ^ M ^ (M & ROL64(b,11))

f(a,b^M,r,i) ^ f(a,b,r,i)
  = ROL64(M,beta) ^ M ^ (a & ROL64(M,11))
```

최상위 비트 XOR는 mod `2^64` 덧셈에서 최상위 비트만 반전한다는 사실과 XOR·회전·AND 전개로 두 식을 도출한다. 각 가변 항은 한 비트이며 독립 균등 `a,b`에서 각 식의 두 출력 차분은 각각 확률 `1/2`다. `r=0,i=0`에서 양쪽 MSB를 동시에 바꾸면 `{0,2^10,2^63,2^63 XOR 2^10}`이 각각 확률 `1/4`다. 항등식 검사는 모든 96개 `(r,i)`에 대해 수행한다. 서로 의존하는 lane 갱신의 확률을 독립 사건처럼 곱하지 않는다.

**Nonce 중복 대조.** 같은 key+nonce+AD로 서로 다른 32바이트 평문을 암호화하면 첫 16바이트에서 `C0 XOR C1 = P0 XOR P1`이 성립하는지 확인한다. 이것은 금지된 nonce 중복의 결과이며 정상적인 고유 nonce 모델의 공격과 분리한다.

### 7.2 알려진 저라운드 신호와 번호 체계

상태 출력의 bit 번호는 LE384 정수의 zero-based LSB-first다. 즉 bit `b`는 byte `b//8`의 bit `b%8`, lane `b//64`의 bit `b%64`다.

이전 감사에서 입력 차분은 **lane5의 bit63**, 즉 전역 bit383만 반전시키고 2라운드 뒤 **전역 출력 bit1**의 차분을 세었을 때 `11/16,384 ≈ 0.0671%`의 반전 빈도가 관측됐다. 새 구현에서 이를 사전 고정한 가설로 재현한다. 이 수치를 새 표본의 강제 기대값으로 사용하지 않는다. P12/P16 전체 공격이나 AEAD 위조로 부풀리지 않는다.

### 7.3 표본과 실행 예산

실험 표본은 다음 형식의 SHAKE256 스트림으로 재현한다.

```text
seed = 0x43574e4f56303926
input = ASCII("CrossWeave/research/") || ASCII(label) || 0x00
        || LE64(seed) || LE32(rounds) || LE32(delta_index) || LE32(dataset_index)
stream = SHAKE256(input, 48 * sample_count)
state_j = stream[48*j : 48*j+48]
```

`label`은 `discovery`, `holdout`, `fixed`, `local`을 사용한다. 서로 다른 label과 dataset index로 탐색·확인 데이터를 분리한다. 선택한 차분과 출력 bit, 표본 수, 생성 입력, 전체 코어 SHA를 JSON에 기록한다.

| 모드 | 확정 실행량 | 판정 |
| --- | --- | --- |
| `research-smoke` | AD 누락·nonce 중복 대조, 모든 `(r,i)`마다 8쌍의 국소 항등식 검사, 고정 2라운드 가설 512쌍 | 정확한 항등식·회귀의 참거짓 검사; 편향은 수치 출력만 |
| `research-full` 국소 | 모든 `(r,i)`마다 256쌍, `r=0,i=0` 동시 MSB 차분 16,384쌍 | 항등식과 지원집합 대조, 빈도 기록 |
| `research-full` 고정 가설 | rounds `{2,4,8,12,16}`, dataset index 0..7 각각 16,384쌍 | 동일 입력 차분·출력 bit의 후속 관측; 40개 사전 고정 검정 |
| `research-full` 탐색 | rounds `{1,2,4,8,12,16}`; 아래 55개 입력 차분마다 1,024쌍, 384개 출력 비트 | 각 라운드의 최대 절대편향 후보 2개 선택 |
| `research-full` holdout | 선택한 12개 후보마다 별도 16,384쌍 | 탐색과 분리한 확인; 선택된 12개 검정군으로 보정 |
| 프레이밍 점검 | 길이 0..33의 AD/메시지 쌍, 각 입력의 모두 0·모두 ff·증가 바이트 패턴 | 실제 흡수 바이트·도메인·빈 최종 블록 사건을 기록하고 의미적 구분 확인 |

55개 차분은 lane0..5 각각 bit `{0,1,7,15,31,32,47,63}` 단독 반전 48개, lane 전체 비트 반전 6개, 전체 384비트 반전 1개다. 후보 동률은 낮은 `delta_index`, 낮은 출력 bit 순서로 선택한다. 키가 없는 순열 실험이므로 존재하지 않는 랜덤 키를 추가하지 않는다.

횟수·반전율·pointwise 99% Wilson 신뢰구간을 남긴다. Wilson 구간에는 `z=2.5758293035489004`를 사용하며 이것을 동시 신뢰구간이라고 부르지 않는다. 귀무가설은 반전 확률 `1/2`이고 양측 이항검정값은 `min(1, 2*sum(comb(n,j), j=0..min(k,n-k))/2^n)`이다. 검정군별 family-wise 기준은 `0.01`로 고정한다. 고정 가설 40개와 선택 holdout 12개를 서로 다른 검정군으로 구분하고, 각각 `min(1,p*40)` 및 `min(1,p*12)`를 보고한다. 탐색 자체의 수많은 출력 비트를 확인용 검정인 것처럼 보고하지 않는다.

통계값 계산은 Python 표준 라이브러리의 정수 또는 log-domain 계산을 사용하고 경계값을 검사한다. 극소 p-value를 0으로 출력할 때 계산 underflow와 실제 확률 0을 혼동하지 않도록 log-p도 남긴다. 통계적 편향 유무를 기본 CI의 무작위 pass/fail 조건으로 쓰지 않는다.

`research-full`은 한 실행당 최대 30분의 실행 예산을 갖고, 각 하위 실험의 완료 수를 저장한다. 시간 초과·중단은 `incomplete`이지 음성 결과나 통과가 아니다. 끝내지 못한 항목은 미완료로 보고하고 이후 사용자가 계속하도록 지정할 수 있다. 대규모 SAT/MILP, 모든 trail/hull, integral·boomerang·MITM·관련키 분석, 모드 전체의 보안 증명은 이번 36개 커밋의 필수 완료 조건이 아니다. 문헌 검토 및 후속 질문으로 남긴다.

### 7.4 보안 결과 분류

각 결과에는 대상 라운드, 입력 선택 능력, 키·nonce 가정, 질의 수, 계산량, 성공률과 영향 범위를 붙인다. `정확한 부품 성질`, `저라운드 구별 신호`, `규격 위반 대조`, `구현 결함`, `미탐색`을 구분한다. P12/P16에서 특정 편향이 안 보였다는 사실만으로 full-round 강도를 선언하지 않는다. AEAD의 다중 사용자·키 수명·전체 위조 한계·프로토콜 적합성은 별도로 미해결 상태를 기록한다.

## 8. 성능과 구현 안전성

### 8.1 측정 항목

동일 호스트·Release 빌드에서 warmup 이후 최소 5개 독립 표본을 수집한다. CPU, OS, 컴파일러와 옵션, OpenSSL 버전, 하드웨어 가속 여부, 샘플 수와 반복 수, 입력 크기, 시간 단위, 중앙값·범위를 함께 저장한다. 중간 결과가 최적화로 제거되지 않도록 결과 checksum을 소비한다. 입력·키 생성 비용은 측정 밖으로 두되, API 자체의 임시 할당·검증·초기화·AD·태그 생성은 전체 AEAD 측정에 포함한다.

- P12/P16 고정 48바이트 호출의 지연.
- AD 0/32/1024바이트 × 메시지 0/16/64/1024/16384/1048576바이트의 전체 암호화와 유효 태그 복호화.
- 인증 실패 복호화의 시간·출력 불변 여부는 별도 집계. 이것만으로 부채널 안전성을 증명하지 않는다.
- 객체 코드 크기와 임시 메모리 상한. 메시지 크기 비례 할당을 숨기지 않는다.
- OpenSSL AES-128-GCM과 ChaCha20-Poly1305의 전체 AEAD 측정. 동일한 AD·메시지 작업량을 사용하되 비교 알고리즘의 정상 key/nonce 길이를 사용한다. ChaCha20의 키 길이·nonce 크기가 다르고 하드웨어 가속 최적화 수준도 다름을 표에 표시한다. 각 알고리즘의 암호화 반복에서 nonce를 중복하지 않는다.

OpenSSL 비교용 `EVP_EncryptUpdate`/`EVP_DecryptUpdate`의 정수 길이 범위도 검사한다. CrossWeave의 한 번 호출 상한은 그 비교 API의 타입 제약을 무시할 근거가 아니다.

이전 감사에서 16KiB 메시지·AD32B의 scalar C 암호화는 약 75.82MiB/s, 16B 메시지는 약 9.86MiB/s였다. 이는 다른 감사 코어·공유 가상화 호스트의 참고값이고 새 구현의 합격선이나 측정 결과가 아니다. 초기 bytewise rate 처리를 두 개 LE64 완전 블록 경로로 최적화할 때는 동일 수식·KAT·부정 테스트·공격 회귀를 유지하고 같은 호스트의 전후 값을 비교한다. 성능 수치에 맞추려고 라운드·태그를 줄이지 않는다.

### 8.2 안전성 검증 범위

ASan/UBSan, 경계·alias·할당 실패, compiler warning, source와 생성 코드의 비밀 의존 분기·주소 접근을 점검한다. `const` 공개 상수 접근과 비밀 인덱스 접근을 구분한다. 입력 길이에 따른 분기는 공개 길이에 따른 것이지만 key·state 비트에 따른 분기는 별도 검토한다. 소스가 분기 없이 보인다는 이유만으로 컴파일된 기계어 전체가 상수시간이라고 단정하지 않는다.

독립 입력으로 동시 호출해 전역 mutable state가 없는지 검사한다. 잘못된 포인터를 일부러 역참조하는 fuzzing을 API 인자 검사라고 부르지 않는다. sanitizer를 실행하지 못한 플랫폼은 미검사로 기록하고 통과로 표시하지 않는다.

## 9. 36개 확정 커밋 계획

아래 번호는 계획과 진행 상태를 찾는 용도다. **실제 커밋 제목에는 번호를 붙이지 않고 제목 열의 문자열을 사용한다.** 제목 접두어는 `feat:`, `fix:`, `test:`, `docs:` 네 가지로 통일한다. `CR-002`, `Commit 1`, `Phase 2`는 실제 제목에 넣지 않는다.

각 행은 의존 순서대로 빌드 가능한 변경과 관련 검증을 함께 담는다. 구현되지 않은 public API가 성공을 반환하는 stub이나 빈 테스트 target은 만들지 않는다. 번호를 맞추기 위한 빈 커밋, 완성 후 인위적인 이력 분할은 금지한다. 필요한 결함 수정 커밋은 `fix:`로 추가하고 계획 번호와 실제 SHA의 대응을 문서에 남긴다.

### Phase 1 — 새 저장소와 공통 기반: 1–5

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| ---: | --- | --- | --- |
| 1 | `docs: define fresh repository scope and execution plan` | 새 대상·범위, 이 사양, README, 진행표, 추적 제외 규칙 | 기존 원격·소스 승계 지시가 없고 계획 1..36이 연속인지 확인; 아직 없는 빌드 성공을 보고하지 않음 |
| 2 | `feat: configure portable cmake build` | C11·OpenSSL 3·CTest·명시 옵션·컴파일러별 경고 설정, 실제 타입/환경 smoke | clean configure/build, `CHAR_BIT==8`, uint64 폭과 OpenSSL 연결을 실행 확인 |
| 3 | `feat: add byte utilities and secure cleanup` | 내부 LE64·회전·주소범위·버퍼 검증·소거 도우미 | 비정렬 바이트 입력, 회전 0/1/63, 범위 overflow·비중첩 경계, 소거 후 메모리 검사 |
| 4 | `test: verify shake256 outputs` | OpenSSL SHAKE256 대조 도우미와 Python 빈 입력·길이별 테스트 | 위 64B KAT, 8/16/136/192B prefix 일치, EVP 실패 경로 확인 |
| 5 | `feat: generate public permutation constants` | RC 32개·IV 생성 도구와 고정 const 배열 | 272바이트 전체 Python/OpenSSL/커밋 배열 일치, NUL·잘못된 도메인 대조 |

### Phase 2 — 순열 코어와 독립 검증: 6–12

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| ---: | --- | --- | --- |
| 6 | `test: add independent permutation oracle` | `tests/reference.py`의 f·라운드·P12/P16·prefix·역변환 | 공개 순열 KAT와 독립 왕복; 기대값에 C 호출을 쓰지 않음 |
| 7 | `feat: implement sequential lane updates` | 내부 C 국소 함수와 `(0,2,4,1,3,5)` 갱신 | overflow를 지나는 값·lane별 중간값·동시 갱신 음성 대조 |
| 8 | `feat: implement round shuffle and fixed permutations` | 상수 XOR·매라운드 재배열·P12/P16 내부 코어 | r=0·r=11·r=15의 중간 상태, 마지막 재배열 포함 KAT |
| 9 | `test: verify inverse rounds and prefix states` | 연구 전용 역라운드·prefix 접근, 테스트 target 분리 | 0..16 prefix의 역변환; 정방향/역방향 index와 재배열 대조 |
| 10 | `test: freeze permutation known answer vectors` | `tests/vectors.json`, zero·ff·증가·80 상태의 고정 벡터 | 고정값을 독립 oracle/C에서 대조, 자동 덮어쓰기 금지 확인 |
| 11 | `feat: expose validated fixed permutation apis` | `crossweave_p384_12/16`, 길이·capacity·NULL·alias 계약 | exact in-place 성공, partial overlap·잘못된 길이 거부, 실패·여유 영역 불변 |
| 12 | `test: cross-check deterministic permutation corpora` | C vector runner와 oracle 비교 도구 | 1,004개 상태의 P12/P16·역변환, 80개 상태×17 prefix 전부 대조 |

### Phase 3 — AEAD와 오류 계약: 13–20

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| ---: | --- | --- | --- |
| 13 | `test: add independent aead oracle` | Python AEAD·태그 검증, 초기화·흡수 trace | 빈 입력 및 17B AD/메시지 고정 KAT; 빈 최종 블록 존재 확인 |
| 14 | `feat: implement initialization and associated data absorption` | C 초기화·key XOR·완전/잔여 AD·도메인 처리 | AD 0/1/15/16/17/31/32/33B의 단계별 state를 oracle과 대조 |
| 15 | `feat: implement ciphertext feedback and finalization` | 내부 메시지 암복호화·암호문 feedback·최종화 | 경계 길이별 내부 결과와 tag; 복호화 state가 평문이 아닌 암호문을 흡수함 확인 |
| 16 | `feat: expose atomic aead encryption` | 확정 `crossweave_encrypt`, 임시 출력과 tag의 성공 후 복사 | 비중첩/in-place KAT, 실패시 ciphertext와 tag 모두 불변, 여유 capacity 보존 |
| 17 | `feat: expose authenticated plaintext release` | `crossweave_decrypt`, 전체 태그 비교·성공 후 평문 공개·소거 | 정상 복호화, 틀린 태그의 평문 미공개, in-place 실패의 입력 암호문 보존 |
| 18 | `test: freeze boundary aead vectors` | 144개 길이 조합 고정 생성·대표 벡터·테스트 | C/Python 암호문·태그·복호화 일치, 빈/정배수 추가 패딩 trace |
| 19 | `test: enforce argument and overlap contracts` | 모든 길이·NULL·capacity·output/input alias 표의 부정 테스트 | 오류 우선순위·양방향 중첩·key/nonce/AD/tag alias·실제 쓰기 구간 경계 검사 |
| 20 | `test: cover allocation and authentication failures` | 내부 테스트 전용 할당 실패 주입·출력 sentinel·정리 검사 | 메시지0 무할당 경로와 비영 길이 할당 실패, 인증 실패 전 경로의 출력 불변·누수 검사 |

### Phase 4 — 회귀와 이식성: 21–25

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| ---: | --- | --- | --- |
| 21 | `test: reject field tampering and truncated tags` | 고정 fixture의 key·nonce·AD·ciphertext·tag 단독 비트 변조 | 총 1,168개 변조의 인증 실패·출력 불변, tag 길이 0/15/17 거부 |
| 22 | `test: add deterministic aead differential corpus` | 256개 추가 사례, `check-differential` target | 400개 AEAD 사례 및 순열 corpus 일괄 대조, 실패시 입력·seed·최초 차이 출력 |
| 23 | `test: add sanitizer and buffer boundary checks` | Linux ASan/UBSan 옵션·guard 영역·최대 길이·반복 호출 검사 | Debug sanitizer 전체 회귀, 상한 `2^24` 유효 처리와 상한 초과의 접근 전 거부 |
| 24 | `test: add linux windows and macos ci` | Linux GCC/Clang, Windows MSVC x64, macOS AppleClang의 실제 build/test | 각 job에서 KAT·부정·독립 대조 실행; Linux sanitizer 별도 job, GCC 플래그의 MSVC 유출 금지 |
| 25 | `docs: document ownership and side-channel limits` | 실제 코드 검토를 반영한 API·소거·분기/주소·동시 호출 책임 | 헤더와 문서 계약 대조, 생성 코드 검토 범위 기록, 비밀 입력/태그 로그 유출 확인 |

### Phase 5 — 재현 가능한 제한 연구: 26–31

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| ---: | --- | --- | --- |
| 26 | `test: reproduce omitted associated data forgery` | 연구 전용 단일 누락 대조와 정상 코어 회귀 | 정상 질의1회 후 AD 치환 수락 대조, 정상 코어 거부, 5개 tail 길이 회귀 |
| 27 | `test: verify local most significant bit relations` | 국소 항등식 도출·96개 위치 검사·동시 MSB 지원집합 | 고정 식 반례0, 독립 입력 가정·조건부 확률 범위 문서화 |
| 28 | `feat: add deterministic differential experiment runner` | 55개 차분·bit 집계·생성 도메인·discovery/holdout·중단 기록·연구 target 연결 | 작은 입력에서 Python/C count 대조, 384bit 번호·동률 처리·seed 분리·예산 검사 |
| 29 | `test: reproduce the two-round output bias` | lane5 bit63→출력 bit1의 사전 고정 실험과 후속 라운드 | 8개 독립 dataset의 빈도·신뢰구간 기록; 과거 11회 강제 일치 금지 |
| 30 | `test: examine framing boundaries and nonce reuse` | 흡수 transcript와 empty/full/tail 프레이밍·nonce 중복 대조 | 지정된 0..33B/3패턴 집합, 도메인 구분·첫블록 XOR 관계·미탐색 범위 |
| 31 | `docs: record bounded permutation and aead research` | `research-smoke/full` 통합 실행 결과, 선정 JSON·보안 결과·후속 질문 | 계획된 실행 수·완료/timeout·검정군 일치, full-round와 저라운드 결론 분리 |

### Phase 6 — 성능과 최종 인계: 32–36

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| ---: | --- | --- | --- |
| 32 | `feat: add end-to-end benchmark harness` | `bench` target, P12/P16·크기별 AEAD·메모리/코드 크기 측정 | warmup·최소5표본·결과 checksum·nonce 고유성·할당/태그 포함 경계 확인 |
| 33 | `test: compare authenticated encryption baselines` | OpenSSL AES-128-GCM·ChaCha20-Poly1305 비교 모듈 | 각 기준 알고리즘 왕복·변조 거부 후 동일 작업량 측정, key/nonce/가속 차이 명시 |
| 34 | `feat: specialize full-block absorption and feedback` | 완전 rate 블록의 LE64 처리 경로, tail은 bytewise 유지 | bit-compatible KAT/대조/부정/공격 회귀와 sanitizer 재통과, 전후 성능 비교 |
| 35 | `docs: publish measured performance and unresolved risks` | 현재 코어 성능 표, 비용·분산·사용 모델, 남은 보안/이식성 문제 | 과거 수치와 현재 값 분리, 환경·명령·표본·코어 SHA 연결, 미측정 항목 표시 |
| 36 | `docs: finalize reproducible research handoff` | README 실행법, 규범/API/연구/성능 문서, 완료 진행표 | 새 디렉터리에서 모든 명령 재현, 로컬·원격 SHA·CI 상태 확인, 공개 범위와 미완료 목록 인계 |

34번 경로가 대상 환경에서 이득을 주지 않거나 불리하면 기존 정확한 코어를 유지하고 실험 결과를 문서화한다. 그 경우 실제 변경에 맞게 `docs: record full-block optimization results`로 제목을 바꾼 사유를 진행표에 적는다. 성능 이득을 가정해 불필요한 최적화 코드를 합치지 않는다.

## 10. 빌드·검증 명령과 CI

개발 환경에는 C11 컴파일러, CMake 3.20 이상, OpenSSL 3.x 개발 헤더·라이브러리, Python 3.11 이상이 필요하다. `find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)`와 `OpenSSL::Crypto`를 사용하며 OS별 절대 경로를 커밋하지 않는다. CMake가 찾은 Python 인터프리터를 실행한다.

전체 개발·검증용 명령:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCRYPTO_BUILD_RESEARCH=ON -DCRYPTO_BUILD_BENCHMARKS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target check-differential
cmake --build build --config Release --target research-smoke
cmake --build build --config Release --target research-full
cmake --build build --config Release --target bench
```

`CRYPTO_BUILD_RESEARCH`와 `CRYPTO_BUILD_BENCHMARKS` 기본값은 OFF다. 연구·벤치 target은 해당 옵션 ON일 때 제공한다. `check-differential`은 `BUILD_TESTING=ON`에서 제공한다. 각 target은 해당 커밋에서 실제 구현된 뒤부터 실행을 요구하고, 그 전에는 존재하지 않는 target을 성공했다고 보고하지 않는다. 기본 CTest는 정확한 회귀 위주이고 긴 full 연구·벤치마크는 자동 포함하지 않는다.

Linux GCC/Clang sanitizer 명령:

```sh
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCRYPTO_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

`CRYPTO_ENABLE_SANITIZERS` 기본값은 OFF다. 지원하지 않는 컴파일러에 켜면 명확히 진단하고 GCC sanitizer 플래그를 MSVC에 넘기지 않는다. Windows 다중 구성 빌드는 `--config Release`, CTest는 `-C Release`를 함께 사용한다. 외부 CI action은 실제 지원되는 버전/커밋을 확인해 고정하고, 의존성 설치 실패와 소스 테스트 실패를 구분한다. 환경 문제 때문에 검사를 생략하면 생략 사실을 남긴다.

각 변경은 로컬 관련 테스트 후 커밋하고, 권한이 있으면 새 원격에 푸시한다. 원격 CI 실패는 원인을 해결하기 전 다음 기능 완료로 넘기지 않는다. 접근권한·외부 환경 장애는 별도 상태로 남긴다. 실패를 숨기려고 테스트를 지우거나 로그를 통과로 바꾸지 않는다. 별도 요청 없이 기존 이력을 reset/rebase/squash하거나 force-push하지 않는다.

## 11. 완료와 보고 기준

36번은 **연구 기준선의 구현·검증·인계 완료**이지 암호학적 안전성의 승인이나 상용 인증이 아니다. 다음 다섯 항목을 구분해 보고한다.

1. 기능: 규범 수식·고정 벡터·독립 대조·입력 및 출력 계약의 실제 결과.
2. 구현 안전성: 실행한 sanitizer·플랫폼·alias·할당 실패·코드 검토 범위와 미검사 항목.
3. 성능: 현재 코어의 실제 표본·사용 조건·비교 한계. 임의의 평균 이상 판정 금지.
4. 암호분석: 정확한 성질과 저라운드 관측, 가정 위반 대조, 실패한 탐색, 미완료 실험.
5. Git: 완료 계획 번호, 마지막 로컬 SHA, 마지막 푸시 SHA, CI 상태, 다음 번호와 장애.

원격 권한·CI 환경·실험 예산 때문에 남은 작업은 완료 수에서 분리한다. 관련 없는 파일이나 저장소를 수정하지 않는다. 라이선스가 이미 있으면 보존하고, 새 루트 라이선스를 소유자 확인 없이 임의로 선언하지 않는다. 외부 의존성의 라이선스 고지는 별도로 정리한다.

최종 인계에는 이 파일 외의 과거 자료가 필요하다는 조건을 붙이지 않는다. 문서·코드·벡터 간 충돌을 발견하면 위치와 재현 결과를 보고하고, 의도한 수식이 확정되기 전에 기대값을 맞추기 위한 변경으로 덮지 않는다.

## 12. 표준과 라이브러리 참고

- [NIST FIPS 202](https://csrc.nist.gov/pubs/fips/202/final): SHAKE256의 표준 정의.
- [OpenSSL 3 EVP digest API](https://docs.openssl.org/3.0/man3/EVP_DigestInit/): SHAKE256과 XOF 출력 호출 방식.
- [RFC 5116](https://www.rfc-editor.org/rfc/rfc5116): AEAD 인터페이스와 nonce·인증 실패의 기본 개념.

이 자료들은 표준 부품과 인터페이스를 위한 참고다. CrossWeave 순열·모드의 보안 증명이나 RFC에 등록된 알고리즘이라는 근거가 아니다.
