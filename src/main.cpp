//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//
//
//				  Hüseyin Burhan Başaran 50866
//					  Fares Qawasmı 50864
//				   Ömer Faruk Demirtaş 50880
//					  Muhammed Farid 50878
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <iostream>
#include <string>
#include "CvControl.hpp"

using namespace std;

int main(){
	CvControl controller;
	controller.startCamera("Camera Screen");
	//controller.startImage("test.png","Image Screen");
	return 0;
}
