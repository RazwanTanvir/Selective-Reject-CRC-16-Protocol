///Developed by team- AlephNull
///CSE50-Data Communication Project
///Selective Reject CRC-16 Implementation
///Team Members: Razwan Ahmed Tanvir (2015-1-60-111),
///              Sharfi Rahman (2015-1-60-134),
///              Anika Tabassum (2015-1-60-136),
///              Moontaha(2015-1-60-)
///Author:       Razwan Ahmed Tanvir
///Email:        razwantanvir8050@gmail.com
///================================================================================================================
///================================================================================================================


#include<bits/stdc++.h>
#include<windows.h>
using namespace std;


#define DATA_LENGTH 500
#define P 11000000000000101
#define WINDOW_LENGTH 4
#define FCS_BIT 16
#define FRAME_SEQUENCE_NO_RANGE 8
#define SEQUENCE_BIT 1
#define FRAME_LENGTH 50
#define DATA_BIT_PER_FRAME FRAME_LENGTH-SEQUENCE_BIT
#define NO_OF_FRAME ((int)(DATA_LENGTH)/((DATA_BIT_PER_FRAME))+1)
#define FRAME_DAMAGE 2
#define FRAME_LOST 1
#define SUCCESS 0


struct FRAME
{
    int sequence_no ;
    int data_in_frame[DATA_BIT_PER_FRAME+FCS_BIT];
    //int fcs ;
}dummy;

//GLOBAL VARIABLES
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int DATA_STREAM[DATA_LENGTH];
FRAME frame[NO_OF_FRAME];
int read_data_stream = 0;
queue <FRAME> frame_queue;
queue <FRAME> Source;
queue <FRAME> Destination ;
queue <FRAME> FramesInDestination;
stack <int>NAK_stack ;
FRAME log_sent_frame[NO_OF_FRAME];
int sent_frame_counter = 0;
int processed_frame_by_destination[100000];
stack <FRAME> sent_frames_by_source;
stack <int> recent_ack ;
bool STOP = false ;
int counter_procesed_frame = 0;


int seq_counter = 0;

void color (unsigned short v) {   /// color function
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hcon, v);
}
void initialize_data_stream(){
    for(int i=0;i<DATA_LENGTH;i++){
        DATA_STREAM[i] = -1;
    }
}
void generate_data(){
    for(int i=0;i<DATA_LENGTH ;i++){
        DATA_STREAM[i] = rand()%2;
    }
}
void show_data_stream ()
{
    cout<<"Given Data Stream: "<<endl;
    for(int i=0;i<DATA_LENGTH;i++){

        if(i%(int)(DATA_BIT_PER_FRAME)==0 && i!=0){
        cout<<endl;
        }
        cout<<DATA_STREAM[i];
    }
    cout<<endl;
}
void showProcessedFramesByDestination(){
    cout<<"CONNECTION STATS:"<<endl;
    cout<<"============================================================================"<<endl;
    cout<<"Processed frames by destination:";
    for(int i=0;i<seq_counter;i++){
        cout<<processed_frame_by_destination[i]<<"   ";
    }
    cout<<endl;

    color(10);
    cout<<endl<<"Total frames received="<<seq_counter<<endl<<endl;
    cout<<"=============================================================================="<<endl;

    color(4);
    cout<<"Disconnecting with source";
            for(int dot=0;dot<5;dot++){
                cout<<".";
                Sleep(500);
            }
    color(10);
    cout<<endl<<endl<<"Process terminated successfully!"<<endl;
    color(7);
}
void initialize_frame(FRAME f){
    f.sequence_no = -1;
    for(int i=0;i<DATA_BIT_PER_FRAME;i++){
        f.data_in_frame[i] = -1 ;
    }
}

void set_sequence(FRAME *f , int seq_no)
{
    //cout<<"calling set sequence"<<endl;
    //cout<<"set_seq: "<<seq_no<<endl;
    f->sequence_no = seq_no % FRAME_SEQUENCE_NO_RANGE ;
}

 void get_fcs(FRAME *f){

    int i,j,keylen,msglen,fcs = 0;

	char input[1000], key[300],temp[300],quot[1000],rem[300],key1[300],tempKey[300];
	for(int i=0;i<DATA_BIT_PER_FRAME;i++){
        if(f->data_in_frame[i]==0){
            input[i]='0';
        }
        else{
            input[i]='1';
        }
	}

    input[DATA_BIT_PER_FRAME] = '\0';
    int len_input = strlen(input);

    //P 16 bit pre determined divisor value 11000000000000101
	 key[0] = '1' ; key[1] = '1' ;key[2] ='0'  ;key[3] = '0' ;key[4] = '0' ;key[5] ='0'  ;key[6] = '0' ;key[7] ='0'  ;
    key[8] ='0'  ;key[9] = '0' ;key[10] =  '0';key[11] ='0'  ;key[12] ='0'  ;key[13] ='0'  ;key[14] = '1'  ;key[15] = '0'  ;key[16] = '1'  ;key[17] = '\0'  ;

	//key[++keycounter] = '\0' ;
	//cout<<"Key value(P):"<<key<<endl;

	keylen=/*strlen(key)*/ 17; //size of P
	//cout<<"Keylen:"<<keylen<<endl;
	msglen=strlen(input);

	strcpy(key1,key);
	for (i=0;i<keylen-1;i++) {
		input[msglen+i]='0';
	}
	for (i=0;i<keylen;i++)
        temp[i]=input[i];
	for (i=0;i<msglen;i++) {
		quot[i]=temp[0];
		if(quot[i]=='0'){
		 for (j=0;j<keylen;j++)
            key[j]='0';
		}
        else{
		 for (j=0;j<keylen;j++)
            key[j]=key1[j];
        }
		for (j=keylen-1;j>0;j--) {
			if(temp[j]==key[j])
                rem[j-1]='0';
			 else
                rem[j-1]='1';
		}
		rem[keylen-1]=input[i+keylen];
		strcpy(temp,rem);
	}
	strcpy(rem,temp);

	for (i=0;i<keylen-1;i++){
	 //printf("%c",rem[i]);
	 int multiplier = 100;
	 if(rem[i]=='1'){
        fcs += 1*multiplier ;
        multiplier /= 10;
	 }
	 else{
        fcs += 0*multiplier ;
        multiplier /= 10;
	 }

	}


    int save_i ;
    for(int i=0;i<len_input;i++){
        if(input[i]=='1')
            f->data_in_frame[i] = 1;
        else
            f->data_in_frame[i] = 0 ;
        save_i = i;
    }
    for(int i=save_i+1,j=0;i<save_i+keylen;i++,j++){
        if(rem[j]=='1')
            f->data_in_frame[i] = 1;
        else
            f->data_in_frame[i] = 0;
    }

 }


 ///getting the remainder of a frame
int check_error(FRAME *f){

    int i,j,keylen,msglen,fcs = 0;

	char input[10000], key[300],temp[300],quot[1000],rem[300],key1[300],tempKey[300];

	for(int i=0;i<DATA_BIT_PER_FRAME+FCS_BIT;i++){
        if(f->data_in_frame[i]==0){
            input[i]='0';
        }
        else{
            input[i]='1';
        }
	}

    input[DATA_BIT_PER_FRAME+FCS_BIT] = '\0';
    int len_input = strlen(input);
	 key[0] = '1' ; key[1] = '1' ;key[2] ='0'  ;key[3] = '0' ;key[4] = '0' ;key[5] ='0'  ;key[6] = '0' ;key[7] ='0'  ;
    key[8] ='0'  ;key[9] = '0' ;key[10] =  '0';key[11] ='0'  ;key[12] ='0'  ;key[13] ='0'  ;key[14] = '1'  ;key[15] = '0'  ;key[16] = '1'  ;key[17] = '\0'  ;

	//key[++keycounter] = '\0' ;
	//cout<<"Key value(P):"<<key<<endl;

	keylen=/*strlen(key)*/ 17;
	//cout<<"Keylen:"<<keylen<<endl;
	msglen=strlen(input);
	//cout<<"msglen:"<<msglen<<endl;

	strcpy(key1,key);
	for (i=0;i<keylen-1;i++) {
		input[msglen+i]='0';
	}
	for (i=0;i<keylen;i++)
	 temp[i]=input[i];
	for (i=0;i<msglen;i++) {
		quot[i]=temp[0];
		if(quot[i]=='0')
		 for (j=0;j<keylen;j++)
		 key[j]='0'; else
		 for (j=0;j<keylen;j++)
		 key[j]=key1[j];
		for (j=keylen-1;j>0;j--) {
			if(temp[j]==key[j])
			 rem[j-1]='0'; else
			 rem[j-1]='1';
		}
		rem[keylen-1]=input[i+keylen];
		strcpy(temp,rem);
	}
	strcpy(rem,temp);

	for (i=0;i<keylen-1;i++){
	 //printf("%c",rem[i]);
	 int multiplier = 100;
	 if(rem[i]=='1'){
        fcs += 1*multiplier ;
        multiplier /= 10;
	 }
	 else{
        fcs += 0*multiplier ;
        multiplier /= 10;
	 }

	}
    //cout<<"FCS value:"<<fcs<<endl;

	return fcs ;
 }


void set_data(FRAME *f , int i){
    bool val = true;
    for(int j=0;j<DATA_BIT_PER_FRAME;j++){
            f->data_in_frame[j] = DATA_STREAM[read_data_stream++];
            if(read_data_stream>DATA_LENGTH){
                //f->data_in_frame[j] =-1;
                //val = false ;
                //return;
            }

        }


        get_fcs(f);
}

void generate_frames(){

    for(int i=0;i<NO_OF_FRAME;i++){
        //initialize_frame(frame[i]);
        set_sequence(&frame[i], i );
        set_data(&frame[i],i);
        frame_queue.push(frame[i]);
    }
}

void ShowAllFrames (){
    for(int i=0;i<NO_OF_FRAME;i++){
        cout<<"Sequence No:"<<frame[i].sequence_no<<endl;
        cout<<"Data in frame "<<i<<":";
        for(int j = 0;j<DATA_BIT_PER_FRAME && frame[i].data_in_frame[i]!=-1;j++){
            cout<<frame[i].data_in_frame[j];
        }
        //cout<<"FCS:"<<frame[i].fcs;
        cout<<endl;
    }
    cout<<endl<<endl<<endl;
}
void ShowFrameQueue()
{
    queue <FRAME> temp ;
    temp = frame_queue;
    while(!temp.empty()){
       // FRAME f =
        cout<<temp.front().sequence_no<<"->" ;
        for(int i=0;i<DATA_BIT_PER_FRAME+FCS_BIT && temp.front().data_in_frame[i]!=-1;i++){
        cout<<temp.front().data_in_frame[i];
        }
        //cout<<"("<<temp.front().fcs<<")";
        temp.pop();
        cout<<endl;

    }
}

void initialize_log_sent_frame(){
    for(int i=0;i<NO_OF_FRAME;i++){
        log_sent_frame[i] = dummy ;
    }
}


int propagation_state(FRAME *f){
    int value = rand()%10 ;
    /*if(value<1){
        return FRAME_LOST;//frame lost
    }*/
    int value2 = rand()%10;
    if(value2<3){
        return FRAME_DAMAGE ; //frame damaged
    }
    return SUCCESS;

}
void introduce_damage(FRAME *f){
    int value = rand()%((int)(DATA_BIT_PER_FRAME/10));
    int select_position_to_damage;
    for(int i=0;i<value;i++){
        select_position_to_damage = rand()%20 ;
        f->data_in_frame[select_position_to_damage] = (-(~f->data_in_frame[select_position_to_damage]))%2 ;
    }

}

void send_nak_frame(){
    int seq = NAK_stack.top();
    cout<<"SOURCE: "<<"Frame "<<seq<<" Sending again."<<endl;
    NAK_stack.pop();
    stack<FRAME> temp;
    temp = sent_frames_by_source ;

    for(int i=0;temp.size()!=0;i++){
            //cout<<"Send NAK LOOP"<<endl;
        if(temp.top().sequence_no==seq){
            //cout<<"Sending the frame "<<temp.top().sequence_no<<" to destination."<<endl;
            Destination.push(temp.top());
            //cout<<"Frame "<<temp.top().sequence_no<<" Pushed in Destination."<<endl;
            break;

        }
        temp.pop();


    }
}

bool received_NAK(){
    if(NAK_stack.size()!=0){
            //cout<<"NAK received"<<endl;
        return true;
    }
    return false ;
}

void SendFrameFromSource()
{

    FRAME temp;
    if(received_NAK()){
        color(6);
        cout<<"SOURCE: "<<"NAK "<<NAK_stack.top()<<" received by source."<<endl;
        color(7);
        send_nak_frame();
    }
    else{
        temp = Source.front();
        Source.pop();
        sent_frames_by_source.push(temp);
        if(temp.sequence_no==-1){
            return;
        }
        cout<<"SOURCE: "<<"Frame "<<temp.sequence_no<<" left source!"<<endl;

        switch(propagation_state(&temp)) {
        case FRAME_DAMAGE:

            introduce_damage(&temp);
            Destination.push(temp);

            //cout<<"Damaged frame received"<<endl;
            break;
        /*case FRAME_LOST:
            //cout<<"Lost frame!"<<endl;
            break;*/
        case SUCCESS:

            Destination.push(temp);
            //cout<<"Frame "<<temp.sequence_no<<" pushed in destination!"<<endl;
            break;
        default:
            cout<<"Somthing is going wrong here."<<endl;
            break;

        }
    }

}
void ShowSourceFrames(){
    queue <FRAME> temp ;
    temp = Source;
    cout<<"Showing frames in source:"<<endl ;
    while(!temp.empty()){
        cout<<temp.front().sequence_no<<"->" ;
        for(int i=0;i<DATA_BIT_PER_FRAME+FCS_BIT && temp.front().data_in_frame[i]!=-1;i++){
        cout<<temp.front().data_in_frame[i];
        }
        temp.pop();
        cout<<endl;

    }
    cout<<endl<<endl;
}
void ShowDestinationFrames(){
    queue <FRAME> temp ;
    temp = FramesInDestination;
    cout<<endl<<endl<<"Showing reached frames in Destination:"<<endl;
    while(!temp.empty()){
        cout<<temp.front().sequence_no<<"->" ;
        for(int i=0;i<DATA_BIT_PER_FRAME && temp.front().data_in_frame[i]!=-1;i++){
        cout<<temp.front().data_in_frame[i];
        }
        temp.pop();
        cout<<endl;

    }
    cout<<endl<<endl;
}

void config_source(){
    generate_frames();
    Source = frame_queue ; //inserting the frames in source
}
void send_ACK(int ack_no){
    recent_ack.push(ack_no) ;
}
void process_frame(){
    //cout<<"IN PROCESSING FRAME FUNCTION"<<endl;
    queue <FRAME> temp ;
    FRAME received_frame ;
    temp = Destination ;
    int recent_frame_seq;

    int que_size = Destination.size();
    if(que_size!=0){
        received_frame= Destination.front();

        Destination.pop();
         // prev temp.front
        recent_frame_seq = received_frame.sequence_no;
        //cout<<"Processing the frame:"<<recent_frame_seq<<endl;
        if(check_error( &received_frame)==0 ){
            color(10);
            FramesInDestination.push(received_frame);
            cout<<"DESTINATION: "<<"Processed the received frame("<<received_frame.sequence_no<<")"<<endl<<endl<<endl;

            color(7);
            counter_procesed_frame++;
            if(counter_procesed_frame==WINDOW_LENGTH){
                send_ACK((recent_frame_seq+1)%FRAME_SEQUENCE_NO_RANGE);
                counter_procesed_frame=0;
            }

            int serial = abs(processed_frame_by_destination[seq_counter]-recent_frame_seq) ;
            processed_frame_by_destination[seq_counter++] = recent_frame_seq ;
            ///edit this field
            //sent_frame_counter-- ;

    }

    else{
        color(4);
        cout<<"DESTINATION: "<<"Got a damaged frame("<<recent_frame_seq<<")!"<<endl;
        NAK_stack.push(recent_frame_seq);
        cout<<"DESTINATION: "<<"NAK for frame "<<recent_frame_seq<<" sent to source."<<endl;
        color(7);
    }

}
}

void start_connection(){
    for(int i=0; Source.size()!=0 ;i++){

        if(recent_ack.size()!=0){
            color(6);
            cout<<"SOURCE: "<<"ACK "<<recent_ack.top()<<" received!"<<endl;
            color(7);
            recent_ack.pop();
        }
        else{
            SendFrameFromSource();
            //sent_frame_counter++ ;

        }
        process_frame();

    }

}


int main()
{
    processed_frame_by_destination[seq_counter] = 1;
    dummy.sequence_no = -1 ;

    cout<<"Configuring Source and Destination";
    for(int dot=0;dot<5;dot++){
                cout<<".";
                Sleep(500);
            }
            cout<<endl;

    cout<<"No of frame:"<<NO_OF_FRAME<<endl<<endl<<endl;
    srand(time(NULL));

    generate_data();
    show_data_stream();
    initialize_frame(dummy);
    config_source();
    //Source.push(dummy);
    start_connection();
    ShowDestinationFrames();
    color(10);
    showProcessedFramesByDestination();
    color(7);


}
