

# g++ -g -Wall -D GPERFTOOLS_PROFIE -gdwarf-4 fake_rand.cpp main.cpp  main_logicthread.cpp  main_ontimer1ms.cpp  mersenne_rand.cpp ../cpp/timer_helper.cpp -I ../cpp -lpthread -lprofiler -ltcmalloc


g++ -g -Wall -std=c++11 fake_rand.cpp main.cpp  main_logicthread.cpp  main_ontimer1ms.cpp  mersenne_rand.cpp ../cpp/timer_helper.cpp -I ../cpp -lpthread

