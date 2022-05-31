#include <QCoreApplication>
#include <chrono>
#include <thread>
#include <modules/actuator/actuator.h>
#include <modules/vision/vision.h>
#include <cmath>
#include <ctime>

//functions

float futuro(float x0, float y0, float yf, float xf)
{
    //inicializa:
    float dSx, dSy, dt; //variacao no espaco e no tempo
    float Vy, ay; //velocidade e aceleracao em y
    float t, Syf; //tempo, posicao futura da bola em y

    //variacao do espaco, posicao final menos posicao inicial
    dSx = xf - x0;
    dSy = yf - y0;

    std::cout <<" XF: "<< xf <<" x0: "<<x0<< std::endl;
    std::cout <<" dSx: "<<dSx<< std::endl;

    //variacao do tempo, 3x o periodo, ja que eh executado na 3 iteracao
    dt=0.05;

    //velocidade robo
    Vy= dSy/dt;
    Vy= Vy;

    //std::cout <<" Vy: "<< Vy <<" dSx: "<<dSx<< std::endl;

    if(Vy==0) //desaceleracao vem do atrito, se tem movimento desacelera
    {
        ay=0;

    }
    else
    {
        ay= -0.49; //valor calculado apartir da forca de atrito e massa da bola
    }

    t=0.25; //tempo no futuro em que eh calculada a posicao
    Syf= y0 + Vy*t + ay*t*t/2;

    //std::cout <<" dSy: "<< dSy<<" Syf: "<<Syf << std::endl;

    //caso a bola esteja voltando, manda codigo que indica parada
    if(dSx <=0)
    {
        Syf=-100;
    }

    return Syf;
}

//manda o robo para a posicao indicada
void vai(float By,float Ry, Actuator *actuator)
{
    float erro, acp, limite;
    if(By==-100)
    {erro=0;}

    erro=fabs(By-Ry);
    acp=0.020; // taxa de erro aceitavel
    limite= 0.223; //limite das barras do gol
    if( (By>-limite) && (By<limite) ) // so vai atras no limite do gol
    {

        if( (By>Ry) && (erro>acp) )
        {
            actuator->sendCommand(false, 0, 11.0, 11.0);

        }

        if( (By<Ry) && (erro>acp) )
        {
            actuator->sendCommand(false, 0, -11.0, -11.0);

        }

        if( (erro<=acp) )
        {
            actuator->sendCommand(false, 0, 0.0, 0.0);

        }
    }


    else
    {
    actuator->sendCommand(false, 0, 0.0, 0.0);
    }

}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Vision *vision = new Vision("224.0.0.1", 10002);
    Actuator *actuator = new Actuator("127.0.0.1", 20011);

    // Desired frequency
    int desiredFrequency = 60;

    int count=0;
    float Syf =0;
    //inicializa os valores
    float x0, y0,xf, yf;
    xf=0;
    yf=0;
    y0=0;
    x0=0;

    while(true) {
        // TimePoint
        std::chrono::high_resolution_clock::time_point beforeProcess = std::chrono::high_resolution_clock::now();

        // Process vision and actuator commands
        vision->processNetworkDatagrams();

        //bola
        fira_message::Ball ball = vision -> getLastBallDetection();
        //robo amarelo
        fira_message::Robot yellowRobot0= vision -> getLastRobotDetection(true, 0);
        fira_message::Robot yellowRobot1= vision -> getLastRobotDetection(true, 1);
        //robo azul
        fira_message::Robot blueRobot0= vision -> getLastRobotDetection(false, 0);

        xf= ball.x(); //x inicial da bola
        yf= ball.y(); //y inicial da bola


        count=count+1;
        if (count>3)
        {
            Syf = futuro(x0, y0, yf, xf);
            x0=xf;
            y0=yf;
            count =0;
        }

        //funcao faz o robo ir para a posicao futura da bola
        vai(Syf ,blueRobot0.y() , actuator);

        // TimePoint
        std::chrono::high_resolution_clock::time_point afterProcess = std::chrono::high_resolution_clock::now();

        // Sleep thread
        long remainingTime = (1000 / desiredFrequency) - (std::chrono::duration_cast<std::chrono::milliseconds>(afterProcess - beforeProcess)).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
    }

    return a.exec();
}
