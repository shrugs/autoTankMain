#include <StackArray.h>

#include <NewPing.h>
NewPing pl(A4, A5, 200);
NewPing pf(A2, A3, 200);
NewPing pr(A0, A1, 200);

const int dist_after_intersect = 3000;

//changes drift values to lower or raise one tread
int driftAccount = 20;
//turns one tread faster than the other to stabilize turning
int turnAccount = 40;
int turnDelay = 47
\

00;


int nodes[30][4];
StackArray <int> past;



int alreadyWritten[30];
bool hasBeenWritten = false;

int current_intersection = 0;
int prev_intersection = -1;
int next_intersection = 0;
int total_intersections = 1;
int dead_end_intersection;

//direction that the bot will turn
// 0 = back
// 1 = left
// 2 = forward
// 3 = right
int direction = 1;

int l,f,r;

//bool backtracking = false;
bool canTurnL = false;
bool canTurnF = true;
bool canTurnR = false;
const int distance_threshold = 28;


//manual movement code
//remove for production
int time_through = 0;
int directions[][3] = {
  //l,f,r in cm

  {5,30,5},
  {40,5,12},
  {5,12,20},
  {30,5,5},
  {5,5,50},
  {5,5,30},
  {5,5,30},
  {200,200,200}
  
  // {5,20,5},
  // {30,5,30},
  // {30,5,5},
  // {5,5,30},
  // {5,5,5},
  // {200,200,200}

};




void setup() {
  Serial.begin(9600);

    //sets pins as output
  //these are all PWM pins
  //by changing the on/off with a square pwm wave, speed can be averaged to a lower
  //or highher number

  //pins 3, 5 == RIGHT
  //pins 9,10 == LEFT
  pinMode(3,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT); 

  //pwr pins
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT); 

  //distance sensor pins
  //not sure why I named them p, but I did.
  //kidding, it's for "ping" because they're ping sensors
  pinMode(A4,OUTPUT); //pl
  pinMode(A5,INPUT); //pl
  pinMode(A2,OUTPUT); //pf
  pinMode(A3,INPUT); //pf
  pinMode(A0,OUTPUT); //pr
  pinMode(A1,INPUT); //pr

  past.setPrinter(Serial);

  //starts the past[] stack array
  past.push (-1);
  delay(20000);
}

void loop() {
  Serial.println("\n LOOP CALLED");
  Serial.println("Checks distances");



    //if is deadend, backtrack
  //l,f,r will alredy be set by the sensors in the canTurn logic
  //therefore remove this block for production and add sensor code
  // l = directions[current_intersection][0];
  // f = directions[current_intersection][1];
  // r = directions[current_intersection][2];

  //get distances from actual sensors
  l = get_distances('l');
  f = get_distances('f');
  r = get_distances('r');


  Serial.print("left: ");
  Serial.print(l);
  Serial.print(" forward: ");
  Serial.print(f);
  Serial.print(" right: ");
  Serial.println(r);
  // l = 8;
  // f = 13;
  // r = 8;


  //logic to set canTurn stuff
  //uncomment for actual code
  if (l >= distance_threshold) {canTurnL = true;}
    else {canTurnL = false;}

    //can turn forward
  if (f >= distance_threshold) {canTurnF = true;}
    else {canTurnF = false;}

  if (r >= distance_threshold) {canTurnR = true;}
    else {canTurnR = false;}

  Serial.print(canTurnL);
  Serial.print(canTurnF);
  Serial.println(canTurnR);

  //fill in nodes array
  
  //if it's at a dead end (can't turn any direction)
  if (canTurnL == 0 && canTurnF == 0 && canTurnR == 0) {
    
    backtrack();

  }
  //else, continue to move to next intersection
  else {
    logNodes('n');


    //if not a dead end
    printData();
    direction = find_next_intersection();
  



    

    Serial.print("Moving to intersecton: ");
    Serial.print(next_intersection);
    Serial.print(" with direction: ");
    Serial.println(direction);

    //MOVE FOWARD UNTIL F <8
    //OR L AND R > 8
    
    driftAccount = 23;

    if (direction == 1) {
      Serial.println("DIRECTION 1 - left");
      //if the bot should turn left and then go straight
      //turn 90 left
      left(90);
      forward(90);
    }
    else if (direction == 2) {
      //should go straight
      Serial.println("DIRECTION 2 - straight");

      forward(90);
    }
    else if (direction == 3) {
      //turn right 90 degrees
      Serial.println("DIRECTION 3 - right");

      right(90);
      forward(90);
    }
    else if (direction == 0) {
      //turn 1distance_threshold0, drive until intersection, then spin 1distance_threshold0 again
      back(90);
    }

      //while move to next intersection
    //the new prev_intersection is the last current_intersection
    prev_intersection = current_intersection;
    //add that to the past[] stack
    past.push(prev_intersection);
    //set the new current_intersection to the previous next_intersection
    current_intersection = next_intersection;


    Serial.print("total_intersections = ");
    Serial.println(total_intersections);
    Serial.print("current_intersection = ");
    Serial.println(current_intersection);

    //gets to next intersection
    Serial.print("Bot reaches intersection: ");
    Serial.println(next_intersection);
    
  }


  Serial.println("\n\n");
}

void printData() {
  Serial.print("past[] = ");
  Serial.println(past.peek());
  

  Serial.print("total_intersections now: ");
  Serial.println(total_intersections);
  
  Serial.println(current_intersection);
  for (int i = 0; i < 4; ++i)
  {
    Serial.print(nodes[current_intersection][i]);
    Serial.print(",");
  }
  Serial.println();
  

}

void logNodes(char bt) {
  //fills in nodes[] array for the current intersection
  //labels next intersections in l,f,r order
  //total_intersections cannot decrement

  nodes[current_intersection][0] = prev_intersection;

  if (bt == 'b') {
    for (int i = 1; i < 4; ++i)
    {
      /* code */
      nodes[current_intersection][i] = -2;
    }
    Serial.println("lognodes('b') called - setting array to (prev,-2,-2,-2)" );
  }


  else {
  
    if (alreadyWritten[current_intersection] == 1) {
      //has already been written
      hasBeenWritten = true;
    }

    if (!hasBeenWritten) {
      //if it hasn't been written, write to nodes array
      //else do nothing, but the program will think that it has been written

      if (canTurnL) {
        nodes[current_intersection][1] = total_intersections;
         
        total_intersections += 1; 
      }
      else { nodes[current_intersection][1] = -2;}


      if (canTurnF) {
        nodes[current_intersection][2] = total_intersections;
          
        total_intersections += 1;
      }
      else {nodes[current_intersection][2] = -2;}


      if (canTurnR) {
        nodes[current_intersection][3] = total_intersections;
        total_intersections += 1;
        
        
      }
      else {nodes[current_intersection][3] = -2;}

      alreadyWritten[current_intersection] = 1;
    }
  }
}

int find_next_intersection() {
  if (l >= 200 && r >= 200 && f >= 200) {
    Serial.println("YAY GOAL STATE");
    past.push(current_intersection);
    
    while (!past.isEmpty()) {
      Serial.println(past.pop());
    }
    while(true) {

    }
  }

  //choose largest distance as next intersection
  next_intersection = max(l,max(r,f));

  /*
    ADD CODE TO MAKE SURE THAT THE ROBOT DOESN'T GO DOWN A -2 PATH
  */

    //return the direction the bot should go to get to that intersection
    //also set the next_intersection to the number of the next_intersection (node

    //returns next_intersection as the number of the next one
    //and direction as the direction to go
  if (next_intersection == l) {
    next_intersection = nodes[current_intersection][1];
    return 1;
  }
  else if (next_intersection == f) {
    next_intersection = nodes[current_intersection][2];
    return 2;
  }
  else if (next_intersection == r) {
    next_intersection = nodes[current_intersection][3];
    return 3;
  }
}

void backtrack() {
  Serial.println("\n BACKTRACK CALLED");

  logNodes('b');
  //is at a dead end
  Serial.println("HIT DEAD END");
  Serial.println("Robot will now backtrack to previous intersection");
  //current = 3
  //prev = 1
  //past = 0,1
  //dead_end_intersection = NULL

  //set nodes of previous intersection to say that this path is no longer an option
  //also get the direction this intersection was relatibe to the previous_intersection
  
  for (int i = 1; i < 4; ++i) {
    if (nodes[prev_intersection][i] == current_intersection) {
      nodes[prev_intersection][i] = -2;
      direction = i;
    }
  }

 
  


  dead_end_intersection = current_intersection;
  current_intersection = past.pop();
  prev_intersection = past.peek();

  //current_intersection = 1
  //prev_intersection = 0
  //past = 0
  //dead_end_intersection = 3

  //MOVE TO PREVIOUS INTERSECTION
  //move_back_bt(direction) moves the bot back based on which direction
  // it came from
  //1 = prev_intersection's left
  //2 = forward
  //3 = right
  move_back_bt(direction);

  Serial.print("Would MOVE to Intersection ");
  Serial.print(current_intersection);
  Serial.println(" right now");

  Serial.print("current_intersection now: ");
  Serial.println(current_intersection);
  Serial.print("previous_intersection now: ");
  Serial.println(prev_intersection);

  printData();


  //manually find next intersection and move bot to it
  //bot thinks that it is at the previous_intersection right now.
  for (int i = 1; i < 4; ++i)
  {
    if (nodes[current_intersection][i] != -2) {
      next_intersection = nodes[current_intersection][i];
      direction = i;
      break;
    }
  }
  


  // if ll of the nodes of the previous_intersection 
  //( notated by "current_intersection") are blocked, we need to backtrack again
  if (nodes[current_intersection][1] == -2 && 
      nodes[current_intersection][2] == -2 && 
      nodes[current_intersection][3] == -2) {

    next_intersection = nodes[current_intersection][0];
    
    for (int i = 1; i < 4; ++i)
    {
      if (nodes[prev_intersection][i] == current_intersection) {
        direction = i;
      }
    }

    //backtrack again
    backtrack();
  }

  //if not, we only need to backtrack once
  else {
    //I don't think this is final move- should be called every leg of
    Serial.println("FINAL MOVE OF THE BACKTRACK");    
  
  
    //now we have direction and which node is next
    //manually move to that direction
    
    //MOVE FOWARD UNTIL F <8
    //OR L AND R > 8
    driftAccount = 20;

    if (direction == 1) {
      Serial.println("DIRECTION 1 (relative to prev_intersection) - left");
      //if the bot should turn left and then go straight
      //turn 90 left
      hardLeft(90);
      forward(90);
    }
    else if (direction == 2) {
      //should go straight
      Serial.println("DIRECTION 2 (relative to prev_intersection) - straight");
      forward(90);
    }
    else if (direction == 3) {
      //turn right 90 degrees
      Serial.println("DIRECTION 3 (relative to prev_intersection) - right");

      hardRight(90);
      forward(90);
    }
    else if (direction == 0) {
      //turn 180, drive until intersection, then spin 180 again
      back(90);
    }


    prev_intersection = current_intersection;
    //add that to the past[] stack
    past.push(prev_intersection);
    //set the new current_intersection to the previous next_intersection
    current_intersection = next_intersection;



    Serial.print("total_intersections = ");
    Serial.println(total_intersections);
    Serial.print("current_intersection = ");
    Serial.println(current_intersection);

    Serial.print("Bot reaches intersection: ");
    Serial.println(next_intersection);
    

    //maunally reorient bot
    Serial.println("Finished Backtracking.");

  }
    //if the intersection we backtracked to doesn't have any other nodes,
  //backtrack again
  


}

int get_distances(char direction) {
  int prev_dist = 0;
  //if l is requested
  if (direction == 'l'){
    //get distance 
    unsigned int p1_dist = pl.ping_median() / US_ROUNDTRIP_CM; 
    if (p1_dist == NULL) { p1_dist = 200;}
    return p1_dist;
  }

  // if f is requested
  else if (direction == 'f') {
    //get distance
    unsigned int pf_dist = pf.ping_median() / US_ROUNDTRIP_CM;
    if (pf_dist == 0) { pf_dist = 200;}
    return pf_dist;
  }

  else if (direction == 'r'){
    unsigned int pr_dist = pr.ping_median() / US_ROUNDTRIP_CM; 
    if (pr_dist == 0) { pr_dist = 200;} 
    return pr_dist;
  }
}

int get_forward(char direction) {
  int prev_dist = 0;
  // if f is requested
  if (direction == 'f') {
    //get distance
    unsigned int pf_dist = pf.ping_median(10) / US_ROUNDTRIP_CM;
    if (pf_dist == 0) { pf_dist = 200; }
    return pf_dist;
  }
}


void forward(int speed) {

  speed = 90;
  //go forward about a square
  //then check sensors
  unsigned long before = millis();
  while (millis()-before <= 3000) {
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);

    analogWrite(3, speed);
    analogWrite(5, speed);
    analogWrite(9, speed);
    analogWrite(10, speed); 
  }
  //check sensors whie going forward
  while (true) {
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);

    analogWrite(3, speed);
    analogWrite(5, speed);
    analogWrite(9, speed + driftAccount);
    analogWrite(10, speed + driftAccount);


    int l = get_distances('l');
    int f = get_distances('f');
    int r = get_distances('r');
    Serial.print("left: ");
    Serial.print(l);
    Serial.print(" forward: ");
    Serial.print(f);
    Serial.print(" right: ");
    Serial.println(r);
    //if intersection detected
    if ((f < distance_threshold) || (l > distance_threshold || r > distance_threshold) ) {
      break;
    }
    //drift code
    if (l > r) {
      driftAccount = 20;
    }
    else if (r > l) {
      driftAccount = -20;
    }
    else {
      driftAccount = 0;
    }
  }

  stop();

}

void back(int speed) {
  speed = 90;
  //go forward about a square
  //then check sensors
  unsigned long before = millis();
  while (millis()-before <= 3000) {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);

    analogWrite(3, speed);
    analogWrite(5, speed);
    analogWrite(9, speed);
    analogWrite(10, speed); 
  }
  //check sensors whie going forward
  while (true) {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);

    analogWrite(3, speed);
    analogWrite(5, speed);
    analogWrite(9, speed + driftAccount);
    analogWrite(10, speed + driftAccount); 


    int l = get_distances('l');
    int f = get_distances('f');
    int r = get_distances('r');
    Serial.print("left: ");
    Serial.print(l);
    Serial.print(" forward: ");
    Serial.print(f);
    Serial.print(" right: ");
    Serial.println(r);
    //if intersection detected
    if ((f < distance_threshold) || (l > distance_threshold || r > distance_threshold) ) {
      break;
    }
    //drift code
    if (l > r) {
      driftAccount = 20;
    }
    else if (r > l) {
      driftAccount = -20;
    }
    else {
      driftAccount = 0;
    }
  }

  stop();


}


void stop() {
  //sets polarzation of H-Bridge to halt current to motor

  analogWrite(3, 0);
  analogWrite(5, 0);
  analogWrite(9, 0);
  analogWrite(10, 0);
  delay(100);

}


void left(int degree) {

  //turns robot left for specified time duration adjusted by global turnAccount and degree input
  //a turnAccount of 14 seems to work for most surfaces
  hardLeft(90);
  // unsigned int before = millis();
  // int prevL = l;
  // while (millis() - before <= turnDelay) {
  //   //while on the 3 second range
  //   //get forward value
  //   int f = get_forward('f');
  //   //if more than half way though turn and such
  //   if ( (millis() - before > turnDelay/2) && (prevL == f)) {
  //     //if previous left value now equals current forward value
  //     //go ahead and stop turning
  //     break;
  //   }
  //     //left back
  //     digitalWrite(2, LOW);
  //     digitalWrite(4, LOW);
  //     //right fwd
  //     digitalWrite(7, LOW);
  //     digitalWrite(8, LOW);

  //     analogWrite(3, 90);
  //     analogWrite(5, 90);
  //     analogWrite(9, 90);
  //     analogWrite(10, 90); 
  //   //else keep turning like normal
  // }

  // //finally, stop
  // stop();

}


void right(int degree) {
  //same as left but with polarity reversed
//temporarily default to hardRight
  hardRight(90);
  // unsigned int before = millis();
  // int prevR = r;
  // while (millis() - before <= turnDelay) {
  //   //while on the 3 second range
  //   //get forward value
  //   int f = get_forward('f');
  //   //if more than half way though turn and such
  //   if ( (millis() - before > (turnDelay/2)) && (prevR == f)) {
  //     //if previous left value now equals current forward value
  //     //go ahead and stop turning
  //     break;
  //   }
  //   //left fwd
  //   digitalWrite(2, HIGH);
  //   digitalWrite(4, HIGH);
  //   //right back
  //   digitalWrite(7, HIGH);
  //   digitalWrite(8, HIGH);

  //   analogWrite(3, 90);
  //   analogWrite(5, 90);
  //   analogWrite(9, 90);
  //   analogWrite(10, 90); 
    
    
  //   //else keep turning like normal
  // }

  // //finally, stop
  // stop();
}

void hardRight(int degree) {
  //same as left but with polarity reversed


  //left fwd
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);
  //right back
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);

  analogWrite(3, 90);
  analogWrite(5, 90);
  analogWrite(9, 90);
  analogWrite(10, 90); 

  delay(turnDelay);


  //finally, stop
  stop();
}


void hardLeft(int degree) {
  //same as left but with polarity reversed


  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  //right fwd
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);

  analogWrite(3, 90);
  analogWrite(5, 90);
  analogWrite(9, 90);
  analogWrite(10, 90); 

  delay(turnDelay + 1000);


  //finally, stop
  stop();
}

void move_back_bt(int d){
  Serial.println("MOVE BACK BT");
  if (d == 1) {
    Serial.println("Back then readjust right");
    //back then right to readjust
    back(90);
    
    //finally readjust right
    hardRight(90);
  }

  else if (d == 2) {
    Serial.println("Back");
    //just back
    back(90);
  }

  else if (d == 3) {
    //back then left to readjust
    Serial.println("Back then readjust LEFT");
    back(90);
    hardLeft(90);

  } 
}