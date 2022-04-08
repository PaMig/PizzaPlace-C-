//PIZZARIA
#include "pizza.h"

//initiating
int tel = Ntel;
int cook = Ncook;
int ovens = Noven;
int deliverer = Ndeliverer;
int packer = Npacker;
int totalCost = 0;
double maxTime = 0;
double meanWaitTime = 0;
double meanCustomerTime = 0;
double meanOrderTime = 0;
double maxOrderTime = 0;
double meanCoolTime = 0;
double maxCoolTime = 0;
int succ;	//num of successfull payments
int fail;	//num of failed payments

//declaring mutexes
pthread_mutex_t tel_lock,cook_lock,oven_lock,print_lock,T_lock,packer_lock,deliverer_lock,Tcool_lock;

//declaring condition variables
pthread_cond_t tel_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cook_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t oven_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t deliverer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t packer_cond = PTHREAD_COND_INITIALIZER;

void sync_print(const char *format, ...) {
  va_list args;
  va_start(args,format);

  pthread_mutex_lock(&print_lock);
  vprintf(format,args);
  pthread_mutex_unlock(&print_lock);

  va_end(args);
}

//every order is a new thread
double dtime(struct timespec* before,struct timespec* after){
  return (double)((after->tv_sec-before->tv_sec)+(after->tv_nsec-before->tv_nsec)/BILLION);
}

void * order_thread(void *x){
	unsigned int thread_seed;
	int Norder;
	int Tdeliverer;
	int status;//status of this threadq
	int t_id;//the thread id handler
	int order_prep_time;
	int order_pack_time;
	int Cfail;
	double Tpay = 0;
	t_id=(int) x;
	thread_seed=(unsigned) t_id;
	//unique instances for every new thread for start-end
	struct timespec timeOrderStart,timeOrderEnd;
	struct timespec CoolTimeStart,CoolTimeEnd;
	struct timespec timePrepStart,timePrepEnd;
	struct timespec waitStart,waitEnd;


	status=pthread_mutex_lock(&tel_lock);// lock  available telephone
	if (status !=0){
		sync_print("error from pthread_mutex_lock() is %d\n",status);
		exit(-1);
	}
	//Put on hold if theres no telephone available
	if(tel==0){
		//sync_print("\nPut them on hold\n");
		status=pthread_cond_wait(&tel_cond,&tel_lock);
		if (status!=0){
			sync_print("error from pthread_cond_wait() is %d\n", status);
			exit(-1);
			clock_gettime(CLOCK_REALTIME,&waitStart);
		}
	}


	tel=tel-1;
	clock_gettime(CLOCK_REALTIME,&waitEnd);
	double totalOrderTime=dtime(&waitStart,&waitEnd);
	Cfail = rand() % 100; //get Random number until 100
	Norder=rand_r(&thread_seed)%(Norderhigh-Norderlow+1)+Norderlow; //get Random number of pizzas

	status=pthread_mutex_unlock(&tel_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}


	status=pthread_mutex_lock(&cook_lock);// lock  available cook
	if (status !=0){
		sync_print("error from pthread_mutex_lock() is %d\n",status);
		exit(-1);
	}
	//testing while mutex is locked before the wait condition
	while(cook==0){ //we can not lock the cook they are all working

		//testing the predicate
		status=pthread_cond_wait(&cook_cond,&cook_lock);//status:now waiting for a tel
		if (status!=0){
			sync_print("error from pthread_cond_wait() is %d\n", status);
			exit(-1);
		}
	}
	//Pay time and fail Pay chance
	 unsigned int o_seed =+ t_id;
	 Tpay=rand_r(&o_seed)%(Tpaymenthigh-Tpaymentlow+1)+Tpaymentlow; //get random payment time
		sleep(Tpay);


	if (Cfail > 5){
		succ++;
		status=pthread_cond_signal(&oven_cond);
	sync_print("ORDER NO:%d WAS REGISTERED SUCCESSFULLY\n",t_id);
	}else{
		fail++;
		sync_print("ORDER NO:%d FAILED\n",t_id);
	}
	if (status!=0){
		sync_print("error from pthread_cond_signal() is %d\n",status);
		exit(-1);
	}
	tel= tel+1;
	status = pthread_cond_signal(&tel_cond);
	status=pthread_mutex_unlock(&oven_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}
	if(Cfail > 5){
	clock_gettime(CLOCK_REALTIME,&timeOrderStart);
	clock_gettime(CLOCK_REALTIME,&timePrepStart);
	order_prep_time=Norder*Tprep ;
	order_pack_time=Norder*Tpack;
	sleep(order_prep_time);//wait until cook prepares the pizzas for baking
	status=pthread_mutex_lock(&oven_lock);
	if (status!=0){
			sync_print("error from pthread_mutex_lock() is %d\n", status);
			exit(-1);
	}

	//there are cooks available
	cook=cook-1;
	status=pthread_mutex_unlock(&cook_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}
	//One oven for each pizza
	while(ovens < Norder){
		status=pthread_cond_wait(&oven_cond,&oven_lock);
	//	sync_print("Not Enough Ovens");
		if (status!=0){
			sync_print("error from pthread_cond_wait() is %d\n", status);
			exit(-1);
		}
	}

	//there are ovens available
	ovens=ovens-Norder;
	totalCost = totalCost + (Norder * 10);
	status=pthread_mutex_unlock(&oven_lock);

	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}

	//release the cook
	status=pthread_mutex_lock(&cook_lock);
	if(status!=0){
		sync_print("error from pthread_mutex_lock() is %d\n", status);
		exit(-1);
	}

	cook=cook+1;
	status=pthread_cond_signal(&cook_cond);
	if (status!=0){
		sync_print("error from pthread_cond_signal() is %d\n",status);
		exit(-1);
	}

	status=pthread_mutex_unlock(&cook_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n",status);
		exit(-1);
	}
	// wait the pizzas to get baked
	sleep(Tbake);
	status = pthread_mutex_lock(&packer_lock);
	if(status!=0){
		sync_print("error from pthread_mutex_lock() is %d\n",status);
		exit(-1);
	}

	while(packer==0){
		status=pthread_cond_wait(&packer_cond,&packer_lock);
		if(status!=0){
			sync_print("error from pthread_cond_wait() is %d\n",status);
			exit(-1);
		}
	}

	//packer is available
	packer = packer-1 ;
	status=pthread_mutex_unlock(&packer_lock);
	if(status!=0){
		sync_print("error from pthread_cond_wait() is %d\n",status);
		exit(-1);
	}
	//Time of preparation ended
	clock_gettime(CLOCK_REALTIME,&timePrepEnd);
	double o  = dtime(&timePrepStart,&timePrepEnd);
	//Time of cooling is starting
	clock_gettime(CLOCK_REALTIME,&CoolTimeStart);
	order_prep_time=order_prep_time+order_pack_time+Tbake; //add the bake time to the total prep time
	sync_print("ORDER NO:%d WAS PREPARED IN  %.2f MINUTES\n", t_id, o);
	status=pthread_mutex_lock(&deliverer_lock);
	if (status!=0){
			sync_print("error from pthread_mutex_lock() is %d\n", status);
			exit(-1);
		}

	while(deliverer==0){
		status=pthread_cond_wait(&deliverer_cond,&deliverer_lock);
		if (status!=0){
			sync_print("error from pthread_cond_wait() is %d\n", status);
			exit(-1);
		}
	}
	//there are deliverers available
	deliverer=deliverer-1;
	status=pthread_mutex_unlock(&deliverer_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}
	packer=packer+1;
	status=pthread_cond_signal(&packer_cond);
	if(status!=0){
		sync_print("error from pthread_cond_signal() is %d\n",status);
		exit(-1);
	}
	status=pthread_mutex_unlock(&packer_lock);
	if(status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n",status);
		exit(-1);
	}
	//pizzas are baked and given to the deliverer
	status=pthread_mutex_lock(&oven_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_lock() is %d\n", status);
		exit(-1);
	}
	//release ovens
	ovens=ovens+Norder;
	status=pthread_cond_signal(&oven_cond);
	if (status!=0){
		sync_print("error from pthread_cond_signal() is %d\n",status);
		exit(-1);
	}

	status=pthread_mutex_unlock(&oven_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}


	Tdeliverer=rand_r(&thread_seed)%(Tdelhigh-Tdellow+1)+Tdellow;//get Random delivery time
	sleep(Tdeliverer); // wait for deliverer to go
	clock_gettime(CLOCK_REALTIME,&timeOrderEnd);
	clock_gettime(CLOCK_REALTIME,&CoolTimeEnd);
	sleep(Tdeliverer); // wait for deliverer to come back

	//release deliverer
	status=pthread_mutex_lock(&deliverer_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_lock() is %d\n", status);
		exit(-1);
	}

	deliverer=deliverer+1;
	status=pthread_cond_signal(&deliverer_cond);
	if (status!=0){
		sync_print("error from pthread_cond_signal() is %d\n",status);
		exit(-1);
	}

	status=pthread_mutex_unlock(&deliverer_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n", status);
		exit(-1);
	}

	double t=dtime(&timeOrderStart,&timeOrderEnd);
	double coolt=dtime(&CoolTimeStart,&CoolTimeEnd);

	sync_print("ORDER NO:%d WAS DELIVERED IN  %.2f MINUTES\n", t_id, t);

	status=pthread_mutex_lock(&T_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_lock() is %d\n", status);
		exit(-1);
	}
	meanWaitTime+=totalOrderTime;
	if(totalOrderTime >=maxTime) maxTime = totalOrderTime;
	meanOrderTime+=t;
	if (t>=maxOrderTime) maxOrderTime=t;
	status=pthread_mutex_unlock(&T_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n",status);
		exit(-1);
	}

	status=pthread_mutex_lock(&Tcool_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_lock() is %d\n", status);
		exit(-1);
	}

	meanCoolTime+=coolt;

	if (coolt>=maxCoolTime) maxCoolTime=coolt;
	status=pthread_mutex_unlock(&Tcool_lock);
	if (status!=0){
		sync_print("error from pthread_mutex_unlock() is %d\n",status);
		exit(-1);
	}
	}else{
		pthread_mutex_unlock(&cook_lock);
	}
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char ** argv){

	if(argc!=3){
		fprintf(stderr, "error: Provide 2 arguments. Given arguments: %d\n", argc);
		return -1;
	}

	int total_orders;
	total_orders=atoi(argv[1]);

	if(total_orders<=0) {
		fprintf(stderr, "error: Total orders must be positive. Given orders: %d\n",total_orders);
		exit(-1);
	}

	unsigned int seed=(unsigned)atoi(argv[2]);
	assert(seed>0);
	pthread_mutex_init(&tel_lock, NULL);
	pthread_mutex_init(&cook_lock, NULL);
	pthread_mutex_init(&oven_lock, NULL);
	pthread_mutex_init(&packer_lock,NULL);
	pthread_mutex_init(&print_lock, NULL);
	pthread_mutex_init(&deliverer_lock, NULL);
	pthread_mutex_init(&T_lock, NULL);
	pthread_mutex_init(&Tcool_lock, NULL);
	unsigned int t_seed;
	double Torder=0;
	int status;
	int * id_ptr;
	int t_id[total_orders];
	pthread_t threads[total_orders];

	for(int i=0; i<total_orders;i++){

		t_id[i]=i+1;
		id_ptr=(int*)t_id[i];
		status=pthread_create(&threads[i], NULL,order_thread, id_ptr);
		if (status!=0){
			fprintf(stderr, "error from pthread_create() is %d\n", status);
			exit(-1);
		}

		if (i>0) { //we do not wait for the first order
			t_seed=seed+t_id[i];
			Torder=rand_r(&t_seed)%(Torderhigh-Torderlow+1)+Torderlow;
		}
		sleep(Torder);//wait until next order
	}

	for(int i=0;i<total_orders;i++)
	{
	  status=pthread_join(threads[i], NULL);

		if (status!=0){
			fprintf(stderr, "error from pthread_join() is %d\n", status);
			exit(-1);
		}

	}
	printf("\nPROGRAM ENDED THE OUTPUT IS:\n\n");
	printf("NUMBER OF TOTAL SUCCESSFULL ORDERS IS: %d, AND FAILED IS:%d \nTOTAL COST IS: %d\n",succ,fail,totalCost);
	printf("MEAN CUSTOMER WAITING TIME IS:%2.f MINUTES\nMAX CUSTOMER WAITING TIME IS: %2.f MINUTES\n",meanWaitTime/total_orders,maxTime);
	printf("MEAN TOTAL TIME IS %.2f MINUTES \nMAX TOTAL TIME IS %.2f MINUTES\n", meanOrderTime/total_orders, maxOrderTime);
	printf("MEAN COOLING TIME IS %.2f MINUTES\nMAX COOLING TIME IS %.2f MINUTES\n", meanCoolTime/total_orders, maxCoolTime);

	//destroy mutexes and condition variables
	pthread_mutex_destroy(&tel_lock);
	pthread_cond_destroy(&tel_cond);
	pthread_mutex_destroy(&oven_lock);
	pthread_cond_destroy(&oven_cond);
	pthread_mutex_destroy(&cook_lock);
	pthread_cond_destroy(&cook_cond);
	pthread_mutex_destroy(&deliverer_lock);
	pthread_cond_destroy(&deliverer_cond);
	pthread_mutex_destroy(&packer_lock);
	pthread_cond_destroy(&packer_cond);
	pthread_mutex_destroy(&T_lock);
	pthread_mutex_destroy(&Tcool_lock);

	return 0;
}
