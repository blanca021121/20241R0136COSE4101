#!/bin/sh

# 스크립트가 실패할 경우 즉시 종료
set -e

# 컴파일 단계: 모든 C 소스 파일을 오브젝트 파일로 컴파일합니다.
gcc -o scheduler_simulator main.c process.c queue.c scheduler.c metrics.c -lpthread

# 실행 파일을 실행합니다.
./scheduler_simulator

# 실행이 완료되었음을 알리는 메시지 출력
echo "Simulation complete. Check the output above for details."
