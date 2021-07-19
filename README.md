# KPU_SystemProgramming_TeamProject

UNIX 기반 텍스트파일 암호화-복호화 프로그램



### 1. 개발환경
------------------------------


       - 운영체제 : VMware 우분투
       
       - 언어 : 표준 UXIX환경의 C언어
       
       - 협업 도구 : GIT, ZOOM



### 2. 설계도
------------------------------

![image](https://user-images.githubusercontent.com/48792627/126122358-94e230a9-4362-41e5-9f6f-e1955df8be59.png)

서버 : 통신방식이 다른 3개의 서버-클라이언트 를 구현한다.
(message passing, shared memory, pipe)

클라이언트 : 하나의 암호화된 파일을 한 클라이언트의 3개의 스레드가 각각 나누어 가지고, 이를 서버의 스레드와 각각 통신한다.

서버에서 복호화 시키면, 다시 이를 클라이언트의 스레드로 전송하고, 클라이언트는 각 스레드에서 받은 파일을 하나로 합쳐 사용자에게 출력한다.



 
### 3. 시스템 설계
------------------------------

#### Shared Memory

![image](https://user-images.githubusercontent.com/48792627/126122623-b275c9fa-7229-4151-b16c-b0f6360ab459.png)


#### Message Passing

![image](https://user-images.githubusercontent.com/48792627/126122735-3dc66754-1181-4ed4-9a66-e5f2dfaf7584.png)


#### Pipe

![image](https://user-images.githubusercontent.com/48792627/126122786-6162dedc-890b-4049-a43e-ebd809968caa.png)





### 4. 실행 방법
------------------------------

1. SharedMemory

서버 : ./sharedServer
클라이언트 : ./sharedClient


2. MessagePassing

서버 : ./message_passing_server
클라이언트 : ./message_passing_client


3. Pipe

서버 : ./IOModuleServer
클라이언트 : ./IOModuleClient

암호화 되어있는 파일은 code.txt 이며, 
서버에 의해 복호화 되어 터미널에 출력된다.

 
### 5. 실행 화면
------------------------------

![image](https://user-images.githubusercontent.com/48792627/126123462-3429c120-0dc9-40ef-8686-dc8f0867a975.png)




### 6. 현실적 제한 요소
------------------------------

- 같은 파일을 3개로 나누어 한 부분씩 디코딩 후 순서대로 화면에 출력할 때 경쟁상태를 방지하기 위해 pthread_mutex_lock을 사용하여 출력 순서를 정하였다.

- IPC 기법 pipe에서 제공되는 구조체인 struct flock를 이용하여 writelock을 걸어줌으로써 병행성을 높였다.

- pthread 조건 변수인 pthread_cond_t printer(1 ~ 3)를 만들고 파일을 보낸 쓰레드가 보냈던 파일의 복호화된 파일을 받을 수 있도록(순서의 동기화) pthread_mutex_lock을 걸어 조건에 따라서 lock을 풀어주는 등의 순서의 동기화를 위해 사용하였다.



![image](https://user-images.githubusercontent.com/48792627/126123714-8b4f989c-5e49-40eb-8538-e8d1efa4b03d.png)

- 표준 UNIX 환경에서 제공하는 nano 에디터, GCC등을 사용하여 파일을 생성, 편집, 컴파일하여 각각의 IPC 기법을 사용하는 Server, Client를 생성하였다.


- 문자열의 최대 크기를 정하지 않아 사용자가 입력한 데이터의 크기가 제한된 버퍼의 용량을 넘어버리는 경우에 발생하는 buffer overflow 현상을 막기위해 sprintf() 함수를 snprintf() 함수로 교체하였다.






### 7. 성능 측정
------------------------------


# 이론적 관점

- 공유 메모리 : 
커널의 관여 없이 메모리를 직접 사용하여 속도가 빠르다. 프로그램 레벨에서 통신 기능을 제공하여, 자유로운 통신이 가능하다.

- 메시지 패싱 : 
커널을 경유하므로, 속도가 느리다. 해당 프로세스 입장에서 일종의 입출력(I/O)으로 볼 수 있으며 문맥교환이 자주 발생한다.
 
- PIPE : 
반이중 통신으로 상호간의 통신을 위해서는 읽기, 쓰기전용 pipe를 모두 만들어 주어야 한다. 간단하게 사용할 수 있다는 장점이 있고 단순한 데이터 흐름을 가지는 프로그램에 사용하기 용이하다.


# 성능 측정

이론적으로 조사를 해보았을 때 Share Memory의 성능이 가장 뛰어나고 두번째로 Message Passing, 마지막으로 Pipe 순으로 결과가 나올것으로 예상하였다. 그러나 예상과는 달리 Message Passing, Shared Memory, PIPE 순의 성능을 가짐을 알 수 있었다. 현실적 제한요소와 발생할 수 있는 문제점들을 보완하기 위한 코드의 추가로  인해(Ex. Shared Memory while문 추가, PIPE의 단방향성 특성 등) 예상했던 바와 조금 다른 결과물이 나오게 되었다. 이번 팀 프로젝트의 결과물로 성능 측정을 한 결과 3가지의 IPC 기법 중 Message Passing이 가장 뛰어난 성능을 보여주었다. (실전과 이론의 일치하지 않는 부분을 알 수 있었다.) 

![image](https://user-images.githubusercontent.com/48792627/126124014-0e002057-1106-4700-a55f-1f9253b59baf.png)






