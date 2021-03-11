#include "TaskNode.h"
#include "Buttons.h"
#include "TemperatureSensor.h"
#include "NTP.h"
#include "TemperatureSensor.h"

#define MAX_TEMPERATURE_TO_PUMP 80
#define QTY_CYCLES_PUMP_TEST 5
#define DURATION_CYCLE_PUMP_TEST 3000
#define PUMP_TIME_ON 360000
#define PUMP_TIME_OFF 180000
#define MAX_TASKS 30

/* Degrees difference from the current temperature to the ideal task 
temperature so that the system can turn the heater on or off*/ 
#define TEMPERATURE_OFFSET 2 

/* interval until post mqtt message */
#define STATUS_TIME_INTERVAL 1000
#define STATUS_TEMPERATURA_TIME_INTERVAL 2000

enum heaterOperation
{
  Auto = 0,
  Manual = 1
};

class Schedule
{
    //////////////////
    // Propriedades // 
    //////////////////
  private:
    bool WaitingButton;
    bool WaitingNextTask;
    int TemperatureOffset;
    bool WarmingUp;
    bool CoolingDown;
    unsigned long StatusTimeControl;
    unsigned long HeaterTimeControl;
    unsigned long PumpTimeControl;
    char Message[200];
    
  public:
  
    // Primeira tarefa da fila
    TaskNode Tasks[30];

    // Unidade de medida de temperatura
    temperatureUnit UserTemperatureUnit;
    
    // Indica o tipo de automação do aquecedor
    heaterOperation HeaterOperationType;
    
    // Indica se o usuário pausou a operação
    bool Paused;

    // Botões
    bool GreenButtonPressed;
    bool RedButtonPressed;

    // Indica se o usuário quer que teste a bomba
    bool UsePumpTest;

    // Indica se a bomba está sendo testada
    bool TestingPump;

    // Indica se o usuário quer usar a bomba de recirculação
    bool UsePump;

    // Indica se a bomba está ligada
    bool IsPumpOn;

    // Indica a quantidade de ciclos de teste da bomba
    int QtyCyclesPumpTest;

    // Controla o tempo durante o ciclo de testes da bomba;
    unsigned long PumpTestTimeControlON;
    unsigned long PumpTestTimeControlOFF;

    // Indica a quantidade total de Tasks para esta braçagem
    int TotalTasks;

    // Indica a tarefa que está sendo executada
    int ActualTask;

    // Indica se a tarefa foi iniciada
    bool TaskRunning;

    // Indica a duração da tarefa
    unsigned long ActualTaskDuration;

    // Indica o pino da bomba
    int PumpPin;

    // Indica o pino do buzzer
    int BuzzerPin;

    // Indica o pino do aquecedor
    int HeaterPin;
    
    // Indica o pino do botao verde
    int GreenButtonPin;
    
    // Indica o pino do botao vermelho
    int RedButtonPin;
    
    // Indica o pino do botao amarelo
    int BlueLeftButtonPin;
    
    // Indica o pino do botao Azul
    int BlueRightButtonPin;

    float ActualTemperature;

    bool NoTasks;
    
    // Status
    String StatusTemperatura;
    String StatusBomba;
    String StatusAquecedor;
    String StatusSolenoideMostura;
    String StatusActualTask;
    String StatusRestandoActualTask;

    //////////////////
    // Construtoras //
    //////////////////
    Schedule();

    //////////////////
    // Métodos      // 
    //////////////////
    void Loop();
    bool WaitingToStart();
    bool CheckForPause();
    //void Loop2();
    void Add(int, char *, unsigned long, bool);
    void DoPumpTest();
    void SetUsePumpTest();
    void PumpSwitch(bool);
    void PumpProcedures();
    void HeaterSwitch(bool);
    void TemperatureProcedures();
    void TaskProcedures();
    void FinishAll();
    void DisplayStatus();
    String TimeFormat(unsigned long);
    void AvisoSonoro();
    
};

// Construtora padrão
Schedule::Schedule()
{
  strcpy(this->Message, "Nothing");
  this->UserTemperatureUnit = Celsius;
  this->Paused = false;
  this->GreenButtonPressed = false;
  this->RedButtonPressed = false;
  this->WaitingButton = false;
  this->WaitingNextTask = false;
  this->TotalTasks = 0;
  this->ActualTask = -1;
  this->NoTasks = true;
  this->TaskRunning = false;
  this->ActualTaskDuration = 0;
  this->UsePumpTest = false;
  this->TestingPump = false;
  this->UsePump = true;
  this->IsPumpOn = false;
  this->PumpTimeControl = 0;
  this->QtyCyclesPumpTest = QTY_CYCLES_PUMP_TEST;
  this->PumpTestTimeControlON = 0;
  this->PumpTestTimeControlOFF = 0;
  this->PumpPin = -1;
  this->BuzzerPin = -1;
  this->HeaterPin = -1;
  this->GreenButtonPin = -1;
  this->RedButtonPin = -1;
  this->BlueLeftButtonPin = -1;
  this->BlueRightButtonPin = -1;
  this->TemperatureOffset = TEMPERATURE_OFFSET;
  this->ActualTemperature = 0;
  this->WarmingUp = true;
  this->CoolingDown = false;
  this->StatusTimeControl = 0;
  this->HeaterTimeControl = 0;
  this->HeaterOperationType = Auto;
  this->StatusTemperatura = "0 °C";
  this->StatusBomba = "Desligada";
  this->StatusAquecedor = "Desligado";
  this->StatusSolenoideMostura = "Fechada";
  this->StatusActualTask = "0/0";
  this->StatusRestandoActualTask = "0/0";
}

// Método responsável por executar as tarefas
void Schedule::Loop()
{
  if (!this->NoTasks)
  {
    if (millis() - this->HeaterTimeControl > STATUS_TEMPERATURA_TIME_INTERVAL)
    {
      this->ActualTemperature = GetTemperature(this->UserTemperatureUnit);
    }

    if (CheckForPause())
    {
      if(!WaitingToStart())
      {
        // Testar a bomba, caso esteja em teste 
        if(this->TestingPump)
        {
          DoPumpTest();
        }
        else
        {
          TemperatureProcedures();
          PumpProcedures();
          TaskProcedures();        
        }
      }
    }     
  }
}

bool Schedule::WaitingToStart()
{
  bool b_return_value = false;

  // Verificar se existem Tasks
  if (this->TotalTasks > 0)
  {
    // Verifica se ainda não começou
    if (this->ActualTask < 0)
    {
      b_return_value = true;
      
      if (this->WaitingButton)
      {
        if (/*CheckResistiveButton(GreenButtonPin, true) ||*/ this->GreenButtonPressed)
        {          
          this->WaitingButton = false;

          // Inicia a primeira tarefa
          this->ActualTask = 0;
          //this->WaitingNextTask = true;

          // verifica se o usuário quer testar a bomba antes de começar
          if (this->UsePumpTest)
          {
            this->QtyCyclesPumpTest = QTY_CYCLES_PUMP_TEST;     
            this->TestingPump = true;
          }
          else
          {
            if (this->UsePump)
            {
              this->IsPumpOn = true;
              PumpSwitch(true);
            }

            sprintf(this->Message, "[1] - %s", this->Tasks[0].Description);
            if(MQTTPublish(TOPIC_MESSAGE, this->Message))
            {
              Serial.printf("MQTTPublish: %s [%s]\n", TOPIC_MESSAGE, this->Message);
            }
          }
        }
      }
      else
      {
        this->WaitingButton = true;
        if(MQTTPublish(TOPIC_MESSAGE, "Pressione o botão verde para começar."))
        {
          Serial.printf("MQTTPublish: %s[Pressione o botão verde para começar.]\n", TOPIC_MESSAGE);
        }      
      }  
    }      

    return b_return_value;
  }
}

bool Schedule::CheckForPause()
{
  // Verificar se o botão vermelho (pausa) foi pressionado
  if (/*CheckResistiveButton(RedButtonPin, true) ||*/ this->RedButtonPressed)
  {
    this->RedButtonPressed = false;

    // Verificar se está pausado
    if (this->Paused)
    {
      this->Paused = false;
      strcpy(this->Message, "Sistema liberado!");
    }
    else
    {
      this->Paused = true;
      strcpy(this->Message, "O sistema está em pausa!");
    }

    Serial.printf("%s\nPressione o botão vermelho para continuar...\n", this->Message);
    if(MQTTPublish(TOPIC_MESSAGE, this->Message))
    {
      Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_MESSAGE, this->Message);
    }      
  }

  return(!this->Paused);
}

void Schedule::Add(int temperatura, char *descricao, unsigned long duracao, bool recirculacao = false)
{
  // Cria um novo nó para adicionar uma tarefa na fila
  this->Tasks[TotalTasks].Temperature = temperatura;
  strcpy(this->Tasks[TotalTasks].Description, descricao);
  this->Tasks[TotalTasks].Duration = duracao * 60 * 1000;
  this->Tasks[TotalTasks].UsePump = recirculacao;

  // Adiciona a nova tarefa no contador de Tasks
  this->TotalTasks++;
  this->NoTasks = false;
}

void Schedule::DoPumpTest()
{
  // Verifica se estamos testando a bomba
  if (this->TestingPump)
  {
    // Verifica se o teste ainda está sendo executado
    if (this->QtyCyclesPumpTest > 0)
    {
      // Verifica se é o primeiro ciclo do teste
      if ((this->PumpTestTimeControlON == 0) &&
          (this->PumpTestTimeControlOFF == 0))
          {
            this->PumpTestTimeControlON = millis();
            PumpSwitch(true);
            if(MQTTPublish(TOPIC_MESSAGE, "Testando a bomba"))
            {
              Serial.printf("MQTTPublish: %s[Testando a bomba!]\n", TOPIC_MESSAGE);
            }
          }
      else
      {
        // Controla a etapa "ligada" do teste
        if (this->PumpTestTimeControlON > 0)
        {
          if (millis() - this->PumpTestTimeControlON > DURATION_CYCLE_PUMP_TEST)
          {
            this->PumpTestTimeControlON = 0;
            this->PumpTestTimeControlOFF = millis();
            PumpSwitch(false);
          }
        }
        else
        {
          // Controla a etapa "desligada" do teste
          if (millis() - this->PumpTestTimeControlOFF > DURATION_CYCLE_PUMP_TEST)
          {
            this->PumpTestTimeControlOFF = 0;
            this->PumpTestTimeControlON = millis();
            this->QtyCyclesPumpTest--;
            if (this->QtyCyclesPumpTest > 0)
            {              
              PumpSwitch(true);
            }
          }
        }
      }
    }
    else
    {
      this->IsPumpOn = true;
      this->TestingPump = false;
      PumpSwitch(true);
      if(MQTTPublish(TOPIC_MESSAGE, "Teste da bomba executado"))
      {
        Serial.printf("MQTTPublish: %s[Teste da bomba executado!]\n", TOPIC_MESSAGE);
        delay(2000);
      }
      
      sprintf(this->Message, "[1] - %s", this->Tasks[0].Description);
      if(MQTTPublish(TOPIC_MESSAGE, this->Message))
      {
        Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_MESSAGE, this->Message);
      }      
    }
  } 
}

void Schedule::SetUsePumpTest()
{
  this->UsePumpTest = true;
}

void Schedule::PumpSwitch(bool _on)
{
  if (_on)
  {
    if (this->ActualTemperature < MAX_TEMPERATURE_TO_PUMP)
    {    
      digitalWrite(this->PumpPin, HIGH);
      this->IsPumpOn = true;
/*      if(MQTTPublish(TOPIC_PUMP_SWITCH, "on"))
      {
        Serial.printf("MQTTPublish: %s[on]\n", TOPIC_PUMP_SWITCH);
      }*/
    }
    else
    {    
      digitalWrite(this->PumpPin, LOW);
      this->IsPumpOn = false;
      if(MQTTPublish(TOPIC_PUMP_SWITCH, "off"))
      {
        Serial.printf("MQTTPublish: %s[off]\n", TOPIC_PUMP_SWITCH);
      }

      if(MQTTPublish(TOPIC_MESSAGE, "BOMBA - Temperatura Máxima"))
      {
        Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_MESSAGE, "BOMBA - Temperatura Máxima");
      }      
    }
  }
  else
  {
    digitalWrite(this->PumpPin, LOW);
    this->IsPumpOn = false;
    /*if(MQTTPublish(TOPIC_PUMP_SWITCH, "off"))
    {
      Serial.printf("MQTTPublish: %s[off]\n", TOPIC_PUMP_SWITCH);
    }*/
  }
      
  this->PumpTimeControl = millis();  
}

void Schedule::PumpProcedures()
{
  if (this->UsePump)
  {
    if ((this->ActualTask >= 0) && (this->ActualTask < this->TotalTasks))
    {
      if (this->WaitingNextTask)
      {
        if (this->IsPumpOn)
        {
          PumpSwitch(false);
        }
      }
      else
      {
        if (this->IsPumpOn)
        {
          if (millis() - PumpTimeControl > PUMP_TIME_ON)
          {
            PumpSwitch(false);
          }
        }
        else
        {
          if (millis() - PumpTimeControl > PUMP_TIME_OFF)
          {
            PumpSwitch(true);
          }
        }
      }
    }
  }
}

void Schedule::HeaterSwitch(bool _on)
{
  if (_on)
  {
    digitalWrite(this->HeaterPin, HIGH);
    this->WarmingUp = true;
/*    if(MQTTPublish(TOPIC_HEATER_SWITCH, "on"))
    {
      Serial.printf("MQTTPublish: %s[on]\n", TOPIC_HEATER_SWITCH);
    }*/
  }
  else
  {
    digitalWrite(this->HeaterPin, LOW);
    this->WarmingUp = false;
/*    if(MQTTPublish(TOPIC_HEATER_SWITCH, "off"))
    {
      Serial.printf("MQTTPublish: %s[off]\n", TOPIC_HEATER_SWITCH);
    }*/
  }    
}

void Schedule::TemperatureProcedures()
{
/*  if (millis() - this->HeaterTimeControl > STATUS_TEMPERATURA_TIME_INTERVAL)
  {
    this->ActualTemperature = GetTemperature(this->UserTemperatureUnit);
  }
*/

  if (this->ActualTemperature < this->Tasks[ActualTask].Temperature - this->TemperatureOffset)
  {
    HeaterSwitch(true);
  }

  if (this->ActualTemperature > this->Tasks[ActualTask].Temperature + this->TemperatureOffset)
  {
    HeaterSwitch(false);
  }
}

void Schedule::TaskProcedures()
{
  char buf[640];
  if ((this->ActualTask >= 0) && (this->ActualTask < this->TotalTasks))
  {
    if (this->WaitingNextTask)
    {
      if (CheckResistiveButton(GreenButtonPin, true) || this->GreenButtonPressed) 
      {
        this->GreenButtonPressed = false;
        this->ActualTask++;
        this->WaitingNextTask = false;

        if (this->ActualTask < this->TotalTasks)
        {
          sprintf(buf, "[%d] - %s (aguardando)", this->ActualTask + 1, this->Tasks[ActualTask].Description);
          if(MQTTPublish(TOPIC_MESSAGE, buf))
          {
            Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_MESSAGE, buf);
          }
        }
      }        
    }
    else
    {
      if (this->TaskRunning)
      {
        if (millis() - this->ActualTaskDuration > this->Tasks[ActualTask].Duration)
        {
          sprintf(buf, "[%d] - %s - Encerrada", this->ActualTask + 1, this->Tasks[ActualTask].Description);
          if(MQTTPublish(TOPIC_MESSAGE, buf))
          {
            Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_MESSAGE, buf);
          }
          
          this->TaskRunning = false;

          if (this->ActualTask + 1 < this->TotalTasks)
          {
            this->WaitingNextTask = true;
          }
          else
          {
            FinishAll();
          }
        }      
      }
      else
      {
        if (this->Tasks[ActualTask].Type == byTemperature_and_Time)
        {
          if (this->ActualTemperature >= this->Tasks[ActualTask].Temperature)
          {
            AvisoSonoro();
            this->TaskRunning = true;
            this->ActualTaskDuration = millis();
            Serial.println("Tarefa " + String(this->ActualTask + 1) + " iniciada em " + String(this->ActualTaskDuration));
            sprintf(buf, "[%d] - %s", this->ActualTask + 1, this->Tasks[ActualTask].Description);
            if(MQTTPublish(TOPIC_MESSAGE, buf))
            {
              Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_MESSAGE, buf);
            }
          }
        }
      }
    }
  }
}

void Schedule::FinishAll()
{
  this->NoTasks = true;
  PumpSwitch(false);
  HeaterSwitch(false);

  if(MQTTPublish(TOPIC_MESSAGE, "Todas as tarefas foram executadas"))
  {
    Serial.printf("MQTTPublish: %s[Todas as tarefas foram executadas]\n", TOPIC_MESSAGE);
    delay(2000);
  }
}

void Schedule::DisplayStatus()
{  
  char buf[640];
  // Verifica se pode exibir o status
  if (millis() - StatusTimeControl > STATUS_TIME_INTERVAL)
  {
    // Temperatura da atual
    if (this->ActualTemperature <= 0)
    {
      this->ActualTemperature = GetTemperature(this->UserTemperatureUnit);
    }
    
    sprintf(buf, "%.2f", this->ActualTemperature);
    if(MQTTPublish(TOPIC_TEMPERATURE_STATUS, buf))
    {
      Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_TEMPERATURE_STATUS, buf);
    }

    if (digitalRead(this->HeaterPin) == HIGH)
    {
      if(MQTTPublish(TOPIC_HEATER_SWITCH, "on"))
      {
        Serial.printf("MQTTPublish: %s[on]\n", TOPIC_HEATER_SWITCH);
      }
    }
    else
    {
      if(MQTTPublish(TOPIC_HEATER_SWITCH, "off"))
      {
        Serial.printf("MQTTPublish: %s[off]\n", TOPIC_HEATER_SWITCH);
      }
    }

    if (this->IsPumpOn == true)
    {
      if(MQTTPublish(TOPIC_PUMP_SWITCH, "on"))
      {
        Serial.printf("MQTTPublish: %s[on]\n", TOPIC_PUMP_SWITCH);
      }
    }
    else
    {
      if(MQTTPublish(TOPIC_PUMP_SWITCH, "off"))
      {
        Serial.printf("MQTTPublish: %s[off]\n", TOPIC_PUMP_SWITCH);
      }
    }

    if (!this->NoTasks)
    {
//      Serial.printf("ActualTask: [%d] WaitingNextTask: [%d] TaskRunning: [%d] - Type: [%d]\n", this->ActualTask, this->WaitingNextTask, this->TaskRunning, this->Tasks[ActualTask].Type);
        
      if (this->ActualTask == -1)
      {
        // Temperatura da tarefa
        sprintf(buf, "%.2f", this->Tasks[0].Temperature);
        if(MQTTPublish(TOPIC_TASK_TEMPERATURE_STATUS, buf))
        {
          Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_TASK_TEMPERATURE_STATUS, buf);
        }
  
        // Tempo restante da tarefa atual
        sprintf(buf, "%s", TimeFormat(this->Tasks[0].Duration));
        if(MQTTPublish(TOPIC_TASK_DURATION_STATUS, buf))
        {
          Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_TASK_DURATION_STATUS, buf);
        }
      }
      else
      {
        if ((this->ActualTask >= 0) && (this->ActualTask < this->TotalTasks))
        {
          // Temperatura da tarefa
          sprintf(buf, "%.2f", this->Tasks[this->ActualTask].Temperature);
          if(MQTTPublish(TOPIC_TASK_TEMPERATURE_STATUS, buf))
          {
            Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_TASK_TEMPERATURE_STATUS, buf);
          }
          if(this->TaskRunning)
          {
            // Tempo restante da tarefa atual
            unsigned long restando = abs(this->Tasks[ActualTask].Duration - (millis() - this->ActualTaskDuration));
            sprintf(buf, "%s", TimeFormat(restando));
            if(MQTTPublish(TOPIC_TASK_DURATION_STATUS, buf))
            {
              Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_TASK_DURATION_STATUS, buf);
            }
          }
          else
          {
            sprintf(buf, "%s", TimeFormat(this->Tasks[ActualTask].Duration));
            if(MQTTPublish(TOPIC_TASK_DURATION_STATUS, buf))
            {
              Serial.printf("MQTTPublish: %s[%s]\n", TOPIC_TASK_DURATION_STATUS, buf);
            }
          }
        }
        else
        {
          if (this->ActualTask >= this->TotalTasks)
          {
            // Temperatura da tarefa
            if(MQTTPublish(TOPIC_TASK_TEMPERATURE_STATUS, "0"))
            {
              Serial.printf("MQTTPublish: %s[0]\n", TOPIC_TASK_TEMPERATURE_STATUS);
            }
  
            // Tempo restante da tarefa atual
            if(MQTTPublish(TOPIC_TASK_DURATION_STATUS, "0"))
            {
              Serial.printf("MQTTPublish: %s[0]\n", TOPIC_TASK_DURATION_STATUS);
            }
          }
        }
      }
    }
    else
    {
      // Temperatura da tarefa
      if(MQTTPublish(TOPIC_TASK_TEMPERATURE_STATUS, "0"))
      {
        Serial.printf("MQTTPublish: %s[0]\n", TOPIC_TASK_TEMPERATURE_STATUS);
      }

      // Tempo restante da tarefa atual
      if(MQTTPublish(TOPIC_TASK_DURATION_STATUS, "00:00:00"))
      {
        Serial.printf("MQTTPublish: %s[00:00:00]\n", TOPIC_TASK_DURATION_STATUS, buf);
      }
    }
    StatusTimeControl = millis();  
  }
}

String Schedule::TimeFormat(unsigned long tempo)
{
  unsigned long allSeconds = tempo/1000;
  int Horas = allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int Minutos=secsRemaining/60;
  int Segundos=secsRemaining%60;

  char buf[21];
  sprintf(buf,"%02d:%02d:%02d", Horas, Minutos, Segundos);
  return String(buf);
}

void Schedule::AvisoSonoro()
{
  digitalWrite(BuzzerPin, HIGH);
  delay(1000);
  digitalWrite(BuzzerPin, LOW);
  delay(1000);
}
