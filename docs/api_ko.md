# 공개 API와 메모리 책임

규범 계약은 [spec_ko.md](spec_ko.md) 5절이다. 공개 헤더 `include/crossweave.h`의
네 함수는 고정 P12/P16과 one-shot 암호화/복호화만 제공한다.
순열 state는48B, key/nonce/tag는각16B, rate16B다. 데이터와 AD 각각 상한은2^24B다.
ciphertext 길이는 plaintext와 같고 tag는 별도다. 절단 태그는 지원하지 않는다.

| 검사 순서 | 결과 |
| --- | --- |
| 고정 크기·메시지/AD 상한 위반 | ELENGTH |
| 실제 필요한 출력보다 작은 capacity | ECAPACITY |
| 필수 NULL·표현 불가능한 주소범위 | EINVAL |
| 금지한 실제 읽기/쓰기 구간 겹침 | EOVERLAP |
| 비어 있지 않은 결과의 임시 할당 실패 | ENOMEM |
| 유효한 복호화 인자의16B tag 불일치 | EAUTH |

AD/메시지 입력은 길이0일 때 NULL을 허용한다. 데이터 출력 NULL은 필요한
길이와 capacity가 모두0일 때만 허용한다. key/nonce/tag/순열 입출력은 필수다.
주소 비교는 uintptr_t 구간으로 수행하고 덧셈 overflow를 먼저 거부한다.
capacity 전체가 아니라 실제 접근하는 길이만 overlap 판정에 사용한다.
호출자가 객체의 생존·실제 크기·읽기/쓰기 권한을 보장해야 한다.
거짓 capacity·dangling pointer·동시 입력 변경은 검증 가능한 계약이 아니다.

P12/P16과 AEAD 데이터는 시작 주소가 같은 정확한 in-place를 지원한다.
서로 다른 시작 주소의 부분 중첩은 양방향으로 거부한다. 데이터 출력과
key/nonce/AD의 겹침은 금지한다. 암호화 tag는 모든 입력/출력과 비중첩이어야
한다. 복호화 출력은 tag와도 겹칠 수 없다. 읽기 전용 입력끼리의 겹침은 허용한다.
데이터 바로 뒤의 tag 배치는 두 실제 구간이 분리되면 허용한다.

실패 시 모든 호출자 출력은 불변이며 암호화는 ciphertext와 tag 모두 보존한다.
성공 시 여유 capacity를 보존한다. 내부 결과를 끝까지 계산한 뒤 복사하며
복호화는 CRYPTO_memcmp의 전체16B 비교가 성공한 후에만 평문을 공개한다.
메시지0은 malloc(0)을 호출하지 않는다. 메시지 nB이면 임시 heap nB를 추가
사용하고 성공·인증실패 모두 해제 직전 소거한다. state와 지역 tag도 소거한다.
입력 객체의 소유권은 호출자에게 있고 코어는 영구 컨텍스트를 저장하지 않는다.

키당 nonce의 고유성과 키 수명은 호출자 책임이다. 재전송 방지나 패킷 번호
검사는 제공하지 않는다. 독립된 입력·출력으로 동시 호출할 수 있으며 공유된
가변 입력을 다른 스레드가 변경하지 않아야 한다. 테스트 전용 실패 주입과
역순열은 생산 라이브러리/공개 헤더에 노출하지 않는다.

# 소거·부채널 검토 범위

GCC14.2 x86_64 Release(-O3 -DNDEBUG)의 cw_local/cw_round/cw_permute 및
AEAD 생성 코드를 objdump로 확인했다. cw_local은 XOR/AND/ADD/회전이고
비밀 값에 따른 분기·테이블 인덱스를 발견하지 않았다. 회전량과 RC 위치는
공개 r/i에 의존한다. AEAD의 루프와 꼬리 처리는 공개 길이, 암복호화 방향,
인자/할당 결과에 의존한다. 태그 비교 결과 분기는 인증 후 공개 여부를 정한다.
생성 코드에서도 CRYPTO_memcmp 호출 뒤에 성공 경로의 memcpy가 위치한다.
이는 제한된 수동 검토로 전체 컴파일러·CPU의 상수시간 증명이 아니다.
OpenSSL 내부 기계어, microarchitecture, 스케줄링·fault·전력 분석은 미검사다.
OPENSSL_cleanse 호출과 메모리0 검사는 레지스터/캐시/외부 복사본의 완전 소거를
보장하지 않는다. 로그나 콜백으로 실제 key/state/평문을 출력하는 생산 경로는 없다.
