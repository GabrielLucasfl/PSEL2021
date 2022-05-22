#include <QCoreApplication>
#include <chrono>
#include <thread>
#include <modules/actuator/actuator.h>
#include <modules/vision/vision.h>
#include <cmath>
//functions

void fazGol(Actuator *actuator){

     actuator->sendCommand(false, 5, 1, 0.0, 0.0, false, 10.0f, false);
}

/*
float VelPegaBola(float Pb, float Pr)

{
    //velocidade em X se as posicoes sao em x, e y para o cont
    float V;
    V= 0.001*fabs(Pb-Pr);

}
*/


//o robo vira para determinado ponto
//robo no ponto p2 vira para alvo no ponto p1

void MiraeChuta(int id, float p1x, float p1y, float p2x, float p2y, float AngRobo,float kick,  Actuator *actuator)
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
            actuator->sendCommand(false, id, 0, 0, 0.6, true, 0, false);
        }

        if ( (AngMin < AngRobo) && (AngRobo < AngMax) )
        {

        actuator->sendCommand(false, id, 0, 0, 0, false, kick);

        }

        if (AngMax < AngRobo)
        {
        actuator->sendCommand(false, id, 0, 0, -0.6, true, 0, false);
        }

}






//o robo vira para determinado ponto
//robo no ponto p1 vira para alvo no ponto p2


int PegaBola(int id, float pbx, float pby, float prx, float pry, float AngRobo,  Actuator *actuator)
{
    int pegou =-1;
    float DeltaX, DeltaY;
    float Ang, AngMin, AngMax;
    DeltaX = pbx - prx;
    DeltaY = pby - pry;

    //angulo que o alvo faz com a reta de refencia
    Ang  = atan2 (DeltaY,DeltaX);

    //intervalo de aceitacao
    AngMin = Ang - 0.02;
    AngMax = Ang + 0.02;

    //velocidade proporcional
    float Vx, Vy;
    Vx= 0.001* fabs(DeltaX);
    Vy= 0.001* fabs(DeltaY);


    if (AngMin > AngRobo)
        {
            actuator->sendCommand(false, id, 0, 0, 0.5);
            pegou =-1;
        }

        if ( (AngMin < AngRobo) && (AngRobo < AngMax) )
        {
            //testa se pegou a bola
            if( ( fabs(DeltaX)< 112.3 ) && ( fabs(DeltaY) <150.3 ) )
            {
                actuator->sendCommand(false, id, 0, 0, 0, true);
                pegou =pegou+1;
                std::cout << " OPA PEDIU PRA PARAR PAROU" << std::endl;
            }
            else
            {
                actuator->sendCommand(false, id, Vx, Vy, 0, true);
            pegou =-1;
            }
        }

        if (AngMax < AngRobo)
        {
        actuator->sendCommand(false, id, 0, 0, -0.5);
        pegou =-1;
        }
        return pegou;
}



int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Vision *vision = new Vision("224.5.23.2", 10020);
    Actuator *actuator = new Actuator("127.0.0.1", 20011);

    // Desired frequency
    int  desiredFrequency = 60;
    int Estado =5;
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
        //robo0 mira no robo2 e chuta
        MiraeChuta(0, blueRobot2.x(), blueRobot2.y(), blueRobot0.x(), blueRobot0.y() ,blueRobot0.orientation(),6.0 , actuator);

        //MAQUINA DE ESTADOS
        if( (Estado !=2) && (Estado !=3) && (Estado !=4) )
        {
            Estado=1;
        }
        switch (Estado)
        {
            //Estado 1
            case 1:
                std::cout << " Estado 1 foi" << std::endl;
                //robo3 mira para a bola
                MiraeChuta(3, ball.x(), ball.y(), blueRobot3.x(), blueRobot3.y() ,blueRobot3.orientation(),0 , actuator);

                //Robo0 pega a bola
                possedeBola= PegaBola(0, ball.x(), ball.y(), blueRobot0.x(), blueRobot0.y() ,blueRobot0.orientation(), actuator);
                //testa se de fato o robo pegou a bola
                if(possedeBola == 0)
                {
                    Estado =2;
                }
            break;

                //Estado 2
            case 2:
                //robo0 mira no robo2 e chuta
                MiraeChuta(0, blueRobot2.x(), blueRobot2.y(), blueRobot0.x(), blueRobot0.y() ,blueRobot0.orientation(),6.0 , actuator);

               //testa se o robo2 pegou a bola
               if(possedeBola== 2)
               {
                    Estado= 3;
               }

            break;

                //Estado 3
            case 3:

                    //robo2 vira para robo3 e chuta
                    MiraeChuta(2, ball.x(), ball.y(), blueRobot0.x(), blueRobot0.y() ,blueRobot0.orientation() ,6.0 , actuator);

                   //testa se o robo3 pegou a bola
                    if(possedeBola== 3)
                    {
                        Estado= 4;
                    }

            break;

            //Estado 4
            case 4:
                //faz o robo ir para a posicao de chute

                //robo3 vira para gol e chuta
                MiraeChuta(3 ,ball.x(), ball.y(), blueRobot0.x(), blueRobot0.y() ,blueRobot0.orientation(),6.0 , actuator);

            break;

            default:
                Estado =1;
            break;

        }
        std::cout <<"Estado: " <<Estado<<" posse de bola: "<< possedeBola << std::endl;


        ///////////////////////////// FIM DA MAQUINA DE ESTADOS
        //actuator->sendCommand(false, 0, Vx, Vy, 0, true);
        //std::cout <<"Angulo robo:"<< blueRobot1.orientation() << "rad Ang Bola: " << std::endl;
        std::cout << "posicao 0roboX:" <<blueRobot0.x() << " 0posicao roboY:" <<blueRobot0.y() << std::endl;
        std::cout << "posicao bolaX:" <<ball.x() << " posicao BolaY:" <<ball.y() << std::endl;

        //end codigo

        // TimePoint
        std::chrono::high_resolution_clock::time_point afterProcess = std::chrono::high_resolution_clock::now();

        // Sleep thread
        long remainingTime = (1000 / desiredFrequency) - (std::chrono::duration_cast<std::chrono::milliseconds>(afterProcess - beforeProcess)).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
    }

    return a.exec();
}
