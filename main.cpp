/*
 *  RPLIDAR
 *  Ultra Simple Data Grabber Demo App
 *
 *  Copyright (c) 2009 - 2014 RoboPeak Team
 *  http://www.robopeak.com
 *  Copyright (c) 2014 - 2018 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header
#define PI 3.1415926535
#define MAX 255

#ifndef _countof // 배열 갯수 count
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x) // delay
#else
#include <unistd.h>
static inline void delay(_word_size_t ms){
    while (ms>=1000){
        usleep(1000*1000);
        ms-=1000;
    };
    if (ms!=0)
        usleep(ms*1000);
}
#endif

using namespace rp::standalone::rplidar;

bool checkRPLIDARHealth(RPlidarDriver * drv)
{
    u_result     op_result;
    rplidar_response_device_health_t healthinfo;


    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("RPLidar health status : %d\n", healthinfo.status);
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return false;
        } else {
            return true;
        }

    } else {
        fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
        return false;
    }
}

//극좌표 -> xy좌표 변환 함수
void coordinates(_u16 theta, _u16 distance, double &xcoor, double &ycoor) // 리턴값 여러개 하고싶을 때, 주소연산자 이용해서 메모리에 저장
{
	//if (distance > 100.0 && distance < 300.0)
	//{
		xcoor = distance*cos((PI*theta) / 180.0);
		ycoor = distance*sin((PI*theta) / 180.0);
	//}
	//else
	//{
	//	xcoor = 0.0;
	//	ycoor = 0.0;
	//}
}

//pwmvalue에 따른 색깔 선택 함수

void color(_u16 *pwmvalue, int &red, int &green, int &blue)
{
	if (pwmvalue <= (_u16*)400)
	{
		blue = MAX;
	}
	else if (pwmvalue <= (_u16*)700)
	{
		green = MAX;
	}
	else
	{
		red = MAX;
	}
}


#include <signal2.h>
#include <conio.h>
#include <math.h>

_u16 *pwmvalue;

bool ctrl_c_pressed;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}

int main(int argc, const char * argv[]) { //argc = argument count(명령행 옵션 갯수) , argv = argument vector(명령행 옵션의 문자열들)


	FILE *fp;
	FILE *fp2;
	const char * opt_com_path = NULL;
	_u32         baudrateArray[2] = { 115200, 256000 };
	_u32         opt_com_baudrate = 0;
	_u32		 theta, distance = 0.0;
	u_result     op_result;
	double xcoor, ycoor, zcoor = 0.0; // 리턴값 여러개로 하기위해 함수 인풋으로 변수의 주소를 받을때, 선언 및 초기화는 포인터가 아닌 일반 변수로 선언후 초기화!!!!!!!!!(헷갈리지 말자)
	int red = 0;
	int green = 0;
	int blue = 0;

	bool useArgcBaudrate = false;

	//printf("Ultra simple LIDAR data grabber for RPLIDAR.\n"           "Version: "RPLIDAR_SDK_VERSION"\n");

	// read serial port from the command line...
	if (argc > 1) opt_com_path = argv[1]; // or set to a fixed value: e.g. "com3"
										//명령행이 1개이상(argc - 1 해줘야 함. 자기자신을 0번지로 가지기 때문, 포트가 1개이상)
										//argv[1]값을 받아온다. 이때, argv[0]은 프로그램 자신의 파일명

	// read baud rate from the command line if specified...
	if (argc > 2)
	{
		opt_com_baudrate = strtoul(argv[2], NULL, 10); // strtoul(정수로 변환할 문자열, 변환하지 못하는 문자열의 시작위치, 변환하고싶은 진수) -> atoi와는 달리 변환하고자하는 진수선택가능
														//& 변환하지못하는곳의 위치출력 가능
														// baudrate 따로 입력 시(명령행 2개이상 -> argc>2), baudrate 값 argv[2]에 불러옴
		useArgcBaudrate = true;
	}

	if (!opt_com_path) { // 명령행이 1개 ->포트가 1개 -> default 값 COM3 이용
#ifdef _WIN32
		// use default com port
		opt_com_path = "\\\\.\\com3";
#else
		opt_com_path = "/dev/ttyUSB0";
#endif
	}

	// create the driver instance
	RPlidarDriver * drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);

	if (!drv) { // create 값 drv가 생성되지 않았을 경우
		fprintf(stderr, "insufficent memory, exit\n");
		exit(-2);
	}

	rplidar_response_device_info_t devinfo;
	bool connectSuccess = false;
	// make connection...
	if (useArgcBaudrate)
	{
		if (!drv) // 메모리 할당 안됐을시
			drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT); // create로 메모리 할당
		if (IS_OK(drv->connect(opt_com_path, opt_com_baudrate)))
		{
			op_result = drv->getDeviceInfo(devinfo);

			if (IS_OK(op_result))
			{
				connectSuccess = true;
			}
			else
			{
				delete drv;
				drv = NULL;
			}
		}
	}
	else
	{
		size_t baudRateArraySize = (sizeof(baudrateArray)) / (sizeof(baudrateArray[0]));
		for (size_t i = 0; i < baudRateArraySize; ++i)
		{
			if (!drv)
				drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
			if (IS_OK(drv->connect(opt_com_path, baudrateArray[i])))
			{
				op_result = drv->getDeviceInfo(devinfo);

				if (IS_OK(op_result))
				{
					connectSuccess = true;
					break;
				}
				else
				{
					delete drv;
					drv = NULL;
				}
			}
		}
	}
	if (!connectSuccess) {

		fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
			, opt_com_path);
		goto on_finished;
	}

	// print out the device serial number, firmware and hardware version number..
	printf("RPLIDAR S/N: ");
	for (int pos = 0; pos < 16; ++pos) {
		printf("%02X", devinfo.serialnum[pos]);
	}

	printf("\n"
		"Firmware Ver: %d.%02d\n"
		"Hardware Rev: %d\n"
		, devinfo.firmware_version >> 8
		, devinfo.firmware_version & 0xFF
		, (int)devinfo.hardware_version);



	// check health...
	if (!checkRPLIDARHealth(drv)) {
		goto on_finished;
	}

	signal(SIGINT, ctrlc);
	int menu_num, key, max_round, wanted_angle1, wanted_angle2;
	char answer;
select1: // menu select

	printf("메뉴를 선택하세요 1. 시작 2. 종료 : ");
	scanf("%d", &menu_num);
	switch (menu_num)
	{
		case 1: 
			{
				printf("시작\n");
				printf("\n회전 수를 입력하세요(계속 돌리려면 -1 을 입력하세요) : ");
				scanf("%d", &max_round);
				if (max_round == 0)
					return 0;
				printf("\n원하는 관측 시작각도를 입력하세요(연속적으로 관측하려면 -1을 입력하세요) : ");
				scanf("%d", &wanted_angle1);
				if (wanted_angle1 != -1)
				{
					printf("\n원하는 관측 종료각도를 입력하세요(연속적으로 관측하려면 -1을 입력하세요) : ");
					scanf("%d", &wanted_angle2);
				}
				printf("\n1~1023 사이의 pwm을 입력하시오 : ");
				scanf("%d", &pwmvalue);

				while (pwmvalue > (_u16*)1023 || (pwmvalue == (_u16*)0))
				{
					fprintf(stderr, "\n잘못 입력했습니다. 다시입력해주세요 : ");
					scanf("%d", &pwmvalue);
				}

				color(pwmvalue, red, green, blue);
				drv->startMotor(pwmvalue);
				drv->startScan(0, 1);
				break;
			}

		case 2:
			{
				printf("종료");
				drv->stop();
				drv->stopMotor();
				return 0;
			}
		default:
			{
				fprintf(stderr, "잘못 입력했습니다. 다시입력해주세요.\n");
				goto select1;
			}

	}
	
	fp = fopen("polar_coordinate.txt", "w");
	fp2 = fopen("coordinate.txt", "w");

	// fetech result and print it out...
	//while (1)
	for(int round =1; ; round++)
	{
		if (round == max_round + 1)
			break;
		if (kbhit())
		{
			key = getch();
			switch (key)
			{
			case 27:
				printf("정지\n");
				drv->stop();
				drv->stopMotor();
				fclose(fp);
				fclose(fp2);
				printf("좌표값 저장하시겠습니까? (y/n)\n");
				answer = getch();
				if (answer == 'n')
				{
					remove("polar_coordinate.txt");
					remove("coordinate.txt");
					printf("저장하지 않았습니다.\n");
				}
				if (answer == 'y')
				{
					printf("저장했습니다.\n");
				}
				return 0;
			case 112:
				printf("일시정지\n 계속하려면 아무 키 입력\n");
				drv->stop();
				drv->stopMotor();
				while (!kbhit());
				drv->startMotor(pwmvalue);
				drv->startScan(0, 1);
				break;
			}
		}
		rplidar_response_measurement_node_t nodes[8192];
		size_t   count = _countof(nodes);

		op_result = drv->grabScanData(nodes, count);

		if (IS_OK(op_result))
		{
			drv->ascendScanData(nodes, count);
			for (int pos = 0; pos < (int)count; ++pos)
			{
			
					printf("%s pos: %d theta: %f Dist: %f Q: %d \n",
					(nodes[pos].sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ? "S " : "  ",
					pos,
					(nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f,
					//(nodes[pos].angle_q6_checkbit)/1.0f,
					nodes[pos].distance_q2 / 4.0f,
					nodes[pos].sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
					fprintf(fp, "%03.2f %08.2f\n", (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f,
					nodes[pos].distance_q2 / 4.0f);
					//coordinates((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f, nodes[pos].distance_q2 / 4.0f, xcoor, ycoor);
				//리턴값 여러개 하기위해서 함수정의 인풋에 주소연산자 붙였을때 함수 선언시에는 인풋에 그냥 일반 변수 이용!!!!!!!!!!!!!!! 헷갈리지 말자!!!
					//fprintf(fp2, "%5.3f %5.3f %5.3f %d %d %d\n", xcoor, ycoor, zcoor, red, green, blue);
					if (wanted_angle1 == -1)
					{
						coordinates((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f, nodes[pos].distance_q2 / 4.0f, xcoor, ycoor);
						fprintf(fp2, "<%dround> %6.4f %6.4f %6.4f %d %d %d\n", round, xcoor, ycoor, zcoor, red, green, blue);
					}
					else if ((((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f) < (wanted_angle1 - 0.5)) || (((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f) > (wanted_angle2 + 0.5)))
					{
						//fprintf(fp2, "");
						continue;
					}
					else
					{
						coordinates((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f, nodes[pos].distance_q2 / 4.0f, xcoor, ycoor);
						fprintf(fp2, "<%dround>%6.4f %6.4f %6.4f %d %d %d\n", round, xcoor, ycoor, zcoor, red, green, blue);
					}
			}
		}

		if (ctrl_c_pressed) {
			drv->stop();
			drv->stopMotor();
			break;
		}

	}
	drv->stop();
	drv->stopMotor();
	fclose(fp);
	fclose(fp2);
	printf("좌표값 저장하시겠습니까? (y/n)\n");
	answer = getch();
	if (answer == 'n')
	{
		remove("polar_coordinate.txt");
		remove("coordinate.txt");
		printf("저장하지 않았습니다.\n");
	}
	if (answer == 'y')
	{
		printf("저장했습니다.\n");
	}
	return 0;

on_finished:
	RPlidarDriver::DisposeDriver(drv);
	drv = NULL;
	return 0;
}

