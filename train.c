#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Station 
{
    int passengers_on_station;
    int free_seats;
    int flag;

    pthread_mutex_t mutex;

    pthread_cond_t train_arrive;
    pthread_cond_t train_complete; 
    pthread_cond_t next_train;
}Station;

typedef struct train_threadArgs
{
    int seatsNum;
    Station * station;
}train_threadArgs;

void station_init(Station * station)
{
    pthread_mutex_init(&station->mutex , NULL);
    pthread_cond_init(&station->train_arrive, NULL);
    pthread_cond_init(&station->train_complete,NULL);
    pthread_cond_init(&station->next_train,NULL);
    station->free_seats=0;
    station->passengers_on_station=0;
    station->flag= 0;
}
//a train arrives in the station and has opened its doors.
void station_load_train(Station *station, int count)
{
    pthread_mutex_lock(&station->mutex);
    //Train already stop at the station.
    while( station->flag == 1)
        pthread_cond_wait(&(station->next_train), &(station->mutex));

    //number of trains entered the station.
    int num_of_trains = 0;

    station->free_seats = count;
    //Raise Flag up to indicate arrival of train.
    station->flag = 1;
    //Increment number of trains.
    num_of_trains++;
    printf("Train arrived in station \n");

    //Signal passengers on the station that the train arrived.
    pthread_cond_broadcast(&(station->train_arrive));

    //Wait while the station's free_seats and passengers_on_station is not yet full.
    while( (station->free_seats > 0)  && (station->passengers_on_station > 0))
        pthread_cond_wait(&(station->train_complete),&station->mutex);
    //train complete and ready to depart.
    station->flag = 0;
    printf("Train leaving station \n");
    //Signal for the next Train to arrive.
    pthread_cond_signal(&(station->next_train));
    //Leave the station.
    pthread_mutex_unlock(&station->mutex);
    
}

//a passenger arrives in a station
void station_wait_for_train(Station *station)
{
    //Try to acquire lock.
    pthread_mutex_lock(&station->mutex);
    //Number of Waiting Passengers increment.
    station->passengers_on_station++;
    printf("Passenger %d waiting in station \n",station->passengers_on_station);

    // wait if there is no train in station or there is a train but no seats.
    while( station->flag == 0 || station->free_seats == 0) 
        pthread_cond_wait(&(station->train_arrive), &(station->mutex));
    printf("Passenger %d seat in train\n",station->passengers_on_station);
    //Number of Waiting Passengers decrement.
    if(station->passengers_on_station > 0)
        station->passengers_on_station--;
}

// passenger seated on train.
void station_on_board(Station *station)
{
    //Number of Available seats decrement.
    if(station->free_seats > 0)
        station->free_seats--;
    //Signal the Train that the passenger seated.
    pthread_cond_signal(&(station->train_complete));
    //Leave the station.
    pthread_mutex_unlock(&station->mutex);
}


void * train_thread(void * args)
{
    train_threadArgs * trainArgs =(struct train_threadArgs*) args;
    station_load_train(trainArgs->station, trainArgs->seatsNum);
}


void * passenger_thread(void * args)
{
    Station* trainStation=(struct Station*)args;
    station_wait_for_train(trainStation);
    station_on_board(trainStation);
}

void main()
{
    Station trainStation;
    station_init(&trainStation);
    int number_of_trains, number_of_passengers;

    printf("# of passengers on station: ");
    scanf("%d",&number_of_passengers);

    printf("# of trains : ");
    scanf("%d",&number_of_trains);

    pthread_t train_threadArr[number_of_trains],pass_threadArr[number_of_passengers];
    train_threadArgs trainArgs[number_of_trains];

    for(int i=0; i<number_of_trains;i++)
    {
        printf("# of seats of train %d : ",i+1);
        scanf("%d",&trainArgs[i].seatsNum);
        trainArgs[i].station = &trainStation;
    }

   // thread creation
    for (int i = 0; i < number_of_passengers; i++)
        pthread_create(&pass_threadArr[i], NULL, passenger_thread, &trainStation);
    for (int i = 0; i < number_of_trains; i++)
        pthread_create(&train_threadArr[i], NULL, train_thread, &trainArgs[i]);
    for (int i = 0; i < number_of_passengers; i++)
        pthread_join(pass_threadArr[i], NULL);
    for (int i = 0; i < number_of_trains; i++)
        pthread_join(train_threadArr[i], NULL);
}

