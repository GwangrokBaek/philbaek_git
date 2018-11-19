# NOTICE
# RPLIDAR A2의 코드 경우, SLAMTEC사에서 제공하는 SDK를 연구실의 목적에 맞게 수정하고 있습니다.
# SMART FARM의 경우, 교내 SW융합 해커톤 대회에서 스마트팜을 주제로 코드를 작성하였습니다.

1. RPLIDAR A2

기존 RPLIDAR A2에서 제공하는 Ultra_simple 코드는 단순히 약 600rpm으로 회전을 하고 데이터를 cmd창에 표시만 할 뿐이었습니다.
이를 연구실의 목적에 맞게 센서를 제어할 수 있기 위해서 다음과 같은 기능을 추가하였습니다.

a) 센서 on/off

<div>
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48693816-fe552600-ec1d-11e8-80d4-3bddc7046770.PNG">
</div>

switch-case문을 이용해 구현했습니다.

b) 센서 pwm제어

<div>
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48694053-99e69680-ec1e-11e8-8aad-a15c42ef47e6.PNG">
</div>

RPLIDAR A2의 경우 최고 모터속도는 1023으로 제한되어있습니다.

* 주의 : 300이하의 pwm으로 설정할 시, 토크력이 부족해 모터가 돌아가지 않는 현상이 있으므로 유의합니다.

c) 데이터 파일처리

<div>
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48694188-fcd82d80-ec1e-11e8-8769-88ba4769b5fe.PNG">
</div>

극좌표와 xy좌표 파일 2개를 생성합니다.

d) 극좌표 값을 x, y 좌표 값으로 전환

<div>
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48694258-39a42480-ec1f-11e8-987b-60b13d39ada2.PNG">
</div>

극좌표 값을 삼각함수 공식을 이용해 x,y 좌표 값으로 변환해주고, pwm의 크기에 따라 RGB값을 달리 해줍니다.

* 주의 : RPLIDAR_A2는 펌웨어에서 거리 값을 u32 즉, unsigned int32 값으로 받기에 소숫점 이하 자리를 표시하지 않습니다. 단위는 mm입니다.

a) setMotorPWM
b) startMotor

<div>
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48694340-6821ff80-ec1f-11e8-8b22-95142c6b0062.PNG">
</div>

위에서 pwm을 제어하기 위해 pwm의 값을 다른 라이브러리에서 계속 이용할 필요가 있었습니다. 따라서 rplidar_driver.cpp에서 이를 call by address 기법을 통해 포인터로 변환해주어 main.cpp에서 rplidar_driver.cpp로 인자를 전달해주었습니다.

* tip : pwm이 낮을수록 정확도가 더욱 높아집니다.

<div>
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48694856-df0bc800-ec20-11e8-8819-18d1533001e1.PNG">
  <img width="600" src="https://user-images.githubusercontent.com/41013930/48694901-f77be280-ec20-11e8-9d9f-8ca6f6520316.PNG">
</div>


2. SMART FARM

아두이노 메가와 여러 센서들을 복합적으로 이용해 자동제어를 구현했습니다. 하드웨어 뿐만 아니라, 오픈소스인 blynk를 이용해 어플리케이션 또한
구현했습니다. 하지만, 와이파이 통신이 아닌 serial 통신 코드만 구현해두었습니다.

기존에 배포되어있는 라이브러리 함수를 수정하거나 그대로 이용하였습니다.
