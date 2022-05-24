#include <QCoreApplication>
#include <chrono>
#include <thread>
#include <modules/actuator/actuator.h>
#include <modules/vision/vision.h>
#include <cmath>
//functions

//MiraeChuta
//MIRA PARA O PONTO ESPECIFICO
void MiraeChuta(int id,float p1x, float p1y, float p2x, float p2y, float AngRobo,  Actuator *actuator)
{
    float DeltaX, DeltaY;
    float Ang, AngMin, AngMax;
    DeltaX = p1x - p2x;
    DeltaY = p1y - p2y;

    //angulo que o alvo faz com a reta de refencia
    Ang  = atan2 (DeltaY,DeltaX);

    //intervalo de aceitacao
    AngMin = Ang - 0.01;
    AngMax = Ang + 0.01;

        if (AngMin > AngRobo)
        {
            actuator->sendCommand(false, id, 0, 0, 0.5, true, 0, false);
        }

        if ( (AngMin < AngRobo) && (AngRobo < AngMax) )
        {
        actuator->sendCommand(false, id, 0, 0, 0, false, 3.0, false);
        }

        if (AngMax < AngRobo)
        {
        actuator->sendCommand(false, id, 0, 0, -0.5, true, 0, false);
        }

}

//pegar a bola

//Mira
void Mira(int id,float p1x, float p1y, float p2x, float p2y, float AngRobo,  Actuator *actuator)
{
    float DeltaX, DeltaY;
    float Ang, AngMin, AngMax;
    DeltaX = p1x - p2x;
    DeltaY = p1y - p2y;

    //angulo que o alvo faz com a reta de refencia
    Ang  = atan2 (DeltaY,DeltaX);

    //intervalo de aceitacao
    AngMin = Ang - 0.03;
    AngMax = Ang + 0.03;

        if (AngMin > AngRobo)
        {
            actuator->sendCommand(false, id, 0, 0, 1.6, true, 0, false);
        }

        if ( (AngMin < AngRobo) && (AngRobo < AngMax) )
        {
        actuator->sendCommand(false, id, 0, 0, 0, false, 0, false);
        }

        if (AngMax < AngRobo)
        {
        actuator->sendCommand(false, id, 0, 0, -1.6, true, 0, false);
        }

}

//posse de bola
int Posse(float pbx, float pby, float prx, float pry)
{
    int pegou=0;
    float DeltaX, DeltaY;
    DeltaX = pbx - prx;
    DeltaY = pby - pry;
    //testa se pegou a bola
    if( ( fabs(DeltaX)< 112.3 ) && ( fabs(DeltaY) <150.3 ) )
    {
        pegou =1;
        std::cout << " OPA PEDIU PRA PARAR PAROU" << std::endl;
    }

    return pegou;
}

///////////////////////////////
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Vision *vision = new Vision("224.5.23.2", 10020);
    Actuator *actuator = new Actuator("127.0.0.1", 20011);

    // Desired frequency
    int  desiredFrequency = 60;

    //estado inicial da maquina de estados
    int Estado =2;

    //posicao em X e Y do gol
    int golx= 4629.8;
    int goly=-25.798;
    //variaveis auxiliares
    int opa=0;
    int possedeBola =-1;

    while(true) {
        // TimePoint
        std::chrono::high_resolution_clock::time_point beforeProcess = std::chrono::high_resolution_clock::now();

        // Process vision and actuator commands
        vision->processNetworkDatagrams();


        //posicao da bola
        SSL_DetectionBall ball = vision->getLastBallDetection();

        //Posicao Robo Amarelo
        SSL_DetectionRobot yellowRobot = vision -> getLastRobotDetection(true, 0);

        //Posicao Robo azul
        SSL_DetectionRobot blueRobot0 = vision -> getLastRobotDetection(false, 0);
        SSL_DetectionRobot blueRobot1 = vision -> getLastRobotDetection(false, 1);
        SSL_DetectionRobot blueRobot2 = vision -> getLastRobotDetection(false, 2);
        SSL_DetectionRobot blueRobot3 = vision -> getLastRobotDetection(false, 3);
        SSL_DetectionRobot blueRobot4 = vision -> getLastRobotDetection(false, 4);

        //CODIGO
        //MAQUINA DE ESTADOS
        switch (Estado)
        {
            //Estado 1: robo 0 pega a bola
            case 1:
                //Robo0 pega a bola


                //testa se o robo0 pegou a bola
                possedeBola = Posse(ball.x(), ball.y(), blueRobot0.x(), blueRobot0.y());
                opa= opa+possedeBola;
                if(opa >= 3)
                {
                    opa=0;
                    Estado = 2;
                }
            break;

            //Estado 2: robo0 entra em posicao
            case 2:
                //robo2 mira para a bola
                Mira(2, ball.x(), ball.y(), blueRobot2.x(), blueRobot2.y() ,blueRobot2.orientation(), actuator);

                //robo0 vai para perto do robo2

                //testa se robo0 ficou em posicao
                if((possedeBola == 0))
                {
                    opa =opa +1;
                    Estado = opa;
                }
            break;


            //Estado 3: robo 0 passa bola para o robo 2
            case 3:
                //robo3 mira para a bola
                Mira(3, ball.x(), ball.y(), blueRobot3.x(), blueRobot3.y() ,blueRobot3.orientation(), actuator);

                //robo2 mira para a bola para pegala
                Mira(2, ball.x(), ball.y(), blueRobot2.x(), blueRobot2.y() ,blueRobot2.orientation(), actuator);

                //robo0 mira no robo2 e chuta
                MiraeChuta(0, blueRobot2.x(),blueRobot2.y(), blueRobot0.x(), blueRobot0.y() ,blueRobot0.orientation(), actuator);

                //testa se o robo2 pegou a bola
                possedeBola = Posse(ball.x(), ball.y(), blueRobot2.x(), blueRobot2.y());
                opa= opa+possedeBola;
                if(opa >= 3)
                {
                    opa=0;
                    Estado = 4;
                }

            break;

            //Estado 4: robo 2 passa a bola para o robo 3
            case 4:
                //robo3 mira para a bola
                Mira(3, ball.x(), ball.y(), blueRobot3.x(), blueRobot3.y() ,blueRobot3.orientation(), actuator);

                //robo2 vira para robo3 e chuta
                MiraeChuta(2, blueRobot3.x(),blueRobot3.y(), blueRobot2.x(), blueRobot2.y() ,blueRobot2.orientation(), actuator);

                possedeBola = Posse(ball.x(), ball.y(), blueRobot3.x(), blueRobot3.y());
                opa= opa+possedeBola;
                //testa se o robo3 pegou a bola
                if(opa >= 3)
                {
                    Estado= 5;
                }

            break;

            //Estado 5: robo 3 chuta a bola para o gol
            case 5:
                //faz o robo ir para a posicao de chute

               //robo3 vira para gol e chuta
               MiraeChuta(3, golx, goly, blueRobot3.x(), blueRobot3.y() ,blueRobot3.orientation(), actuator);
            break;

            default:
                Estado =1;
            break;
        } //fim da maquina de estados
         std::cout << " bola em X" <<ball.x()<<" bola em y:"<< ball.y()<<" OPA: "<<opa<< std::endl;

        //end codigo

        // TimePoint
        std::chrono::high_resolution_clock::time_point afterProcess = std::chrono::high_resolution_clock::now();

        // Sleep thread
        long remainingTime = (1000 / desiredFrequency) - (std::chrono::duration_cast<std::chrono::milliseconds>(afterProcess - beforeProcess)).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
    }

    return a.exec();
}
