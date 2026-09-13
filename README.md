# RISC-V accelerator simulator — Codex revision

기존 PDF 기반 예시를 실행 가능한 C++20 시뮬레이터 모듈로 재구성한 리포다. 이 프로젝트는 CPU, GPU, NPU가 같은 RISC-V instruction을 재해석하는 모델을 쓰지 않는다. RV64 표준 instruction decode와 XVec 가속기 command ABI를 분리하고, CPU는 MMIO doorbell로 queue를 알리며 가속기는 descriptor ring을 소비한다.

## 구성

```text
RV64 base / M instruction (32-bit encoding)
  -> rv64_decoder
  -> custom-2 xvec.doorbell notification
  -> XvecDescriptor 64-byte ABI
  -> XvecQueue ring
  -> GPU / NPU / DSP backend adapter
```

| 경로 | 역할 |
| --- | --- |
| `include/rvsim/rv64_decoder.hpp` | RV64 base/M의 ADD, SUB, MUL, ADDW, SUBW, MULW decode |
| `include/rvsim/xvec_queue.hpp` | 64-byte 정렬 XVec descriptor와 thread-safe ring queue |
| `include/rvsim/pmp_policy.hpp` | M/S/U privilege와 locked PMP rule을 따르는 접근 정책 모델 |
| `tests/rvsim_tests.cpp` | decode, queue FIFO, PMP 경계 검증 |

## ISA와 가속기 ABI 경계

- RV64의 XLEN은 64비트지만 여기서 다루는 base/M instruction encoding은 32비트다.
- 표준 RVV 명령과 독자 가속기 명령을 섞지 않는다. custom-2 opcode는 `xvec.doorbell`이라는 독자 notification으로만 사용한다.
- `XvecDescriptor`는 instruction encoding이 아니라 CPU driver와 accelerator backend가 공유하는 command ABI다.
- Vulkan, NPU runtime, DMA driver는 queue consumer backend로 구현한다. 이 저장소는 null Vulkan handle을 호출하는 예제 코드를 포함하지 않는다.

## 보안 경계

`PmpPolicy`는 PMP 권한 판정 모델이다. 실제 펌웨어는 boot code에서 PMP CSR을 설정하고 locked rule, 필요하면 Smepmp 정책까지 적용해야 한다. secure service는 `ecall` ABI, untrusted pointer의 범위 검사, copy-in/out, 실제 cryptographic provider를 별도 계층으로 구현해야 한다.

## 빌드와 테스트

```sh
cmake -S . -B build -DRVSIM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

PDF 파일은 초기 바이브 코딩 예시로 남겨뒀다. 현재 구현의 규격이나 실행 경로로 사용하면 안 된다.
