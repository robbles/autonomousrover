import java.util.LinkedList;

Rover rover;

Path path;

int startX = 100;
int startY = 100;

/* SETUP */
void setup() {
  size(800, 600);
  smooth();
  frameRate(30);
  background(0);
  rectMode(CENTER);
  
  // Generate a random path
  path = new Path(startX, startY);
  path.addCheckPoint(0, 600, 150);
  path.addCheckPoint(90, 400, 150);
  path.addCheckPoint(180, 600, 150);
  path.addCheckPoint(270, 400, 150);

  rover = new Rover(startX, startY, 0, path);
  
}



/* MAIN LOOP */
void draw() {
  background(50);

  path.draw();

  rover.loop();
  
}



class Rover {
  
  float x;
  float y;
  int heading;
  float speed;
  float turnSpeed;
  float leftWheelSpeed;
  float rightWheelSpeed;
  Path path;
  int pathIndex;
  CheckPoint currentGoal;
  float distance_traveled = 0;
  float distance_offset = 0;
  boolean done;
  boolean deadReckoningFlag;
  boolean radioFlag;

  Rover(float x, float y, int heading, Path path) {
    this.x = x;
    this.y = y;
    this.heading = heading;
    this.speed = 0;
    this.turnSpeed = 4;
    this.path = path;
    this.pathIndex = 0;
    currentGoal = (CheckPoint)path.checkpoints.get(pathIndex++);
    deadReckoningFlag = false;
    radioFlag = false;
  }

  void loop() {
    if(!done) {
      // Check to see if we're facing the right direction
      if(!onTrack()) {
        // First try to get back to center of track
        adjustToPath();

      } 
      // Go full speed until reaching CheckPoint or detecting obstacle(s)
      else if(!nearCheckPoint()) {
        // On track, go to top speed
        accelerateToSpeed(15.0, 0.2);

        if(detectObstacles()) {
          // TODO: tweak out / do something useful
        }
      }
      else {
        brake(0.8);

        if(abs(speed) == 0) {
          // Clear distance traveled
          distance_offset = distance_traveled;
          deadReckoningFlag = radioFlag = false;
          if(path.checkpoints.size() > this.pathIndex) {
            this.currentGoal = (CheckPoint)path.checkpoints.get(pathIndex++);
          } 
          else {
            done = true;
          }
        }	
      }

    }
    physics();

    draw();
  }

  boolean onTrack() {
    return abs(currentGoal.heading - heading) < turnSpeed;
  }

  void adjustToPath() {
    float diff =  abs((heading + 180 -  currentGoal.heading) % 360 - 180);

    if(abs(currentGoal.heading - heading) == diff) {
      heading = int((heading + turnSpeed) % 360);
    }
    else if(abs(heading - currentGoal.heading) == diff) {
      heading = int((heading - turnSpeed) % 360);
    }
  }

  boolean nearCheckPoint() {
    if(dist(this.x, this.y, currentGoal.endX, currentGoal.endY) < currentGoal.radius/2) {
      radioFlag = true;
      //return true;
    }
    if((distance_traveled-distance_offset) >= (currentGoal.distance-currentGoal.radius/2)) {
      deadReckoningFlag = true;
      return true;
    }
    return false;
  }

  boolean detectObstacles() {
    return false;
  }

  void accelerateToSpeed(float speed, float acceleration) {
    if(this.speed < speed) {
      this.speed += acceleration;
    }
  }

  void brake(float deceleration) {
    if(speed > deceleration) {
      speed -= deceleration;
    } 
    else {
      speed = 0;
    }
  }

  void physics() {

    // Move the rover in the right direction based on its speed
    this.x += this.speed * cos(radians(heading));
    this.y += this.speed * sin(radians(heading));

    distance_traveled += speed;
  }

  void draw() {
    pushMatrix();
    translate(x, y);
    rotate(radians(heading));
    // Wheels
    fill(0);
    stroke(255);
    rect(0, -30, 60, 20);
    rect(0, 30, 60, 20);
    
    float track = distance_traveled/2;
    for(int i=0; i<=30; i+=10) {
      float pos = cos((((track+i)%40)/80.0)*TWO_PI);
      line(-30*pos, 20, -30*pos, 40);
      line(-30*pos, -20, -30*pos, -40);
    }
    
    // Cart
    fill(50);
    rect(0, 0, 50, 40);
    fill(20, 30, 20);
    stroke(30, 40, 30);
    rect(0, 0, 40, 30);
    
    // Center
    stroke(70);
    ellipse(0, 0, 5, 5);
    
    // Flag LEDs
    noStroke();
    if(deadReckoningFlag) {
      fill(255, 0, 0);
      rect(20, 10, 5, 5);
    }
    if(radioFlag) {
      fill(0, 255, 0);
      rect(20, -10, 5, 5);
    }
    

    popMatrix();

    noFill();
    stroke(0, 0, 255);
    
    
  }	
}

class CheckPoint {
  float heading;
  float distance;
  float radius;
  float startX, startY;
  float endX, endY;

  CheckPoint(float heading, float distance, float radius) {
    this.radius = radius;
    this.heading = heading;
    this.distance = distance;
  }

  void setAbsolutePosition(float fromX, float fromY) {
    this.endX = fromX + (this.distance * cos(radians(this.heading)));
    this.endY = fromY + (this.distance * sin(radians(this.heading)));
    this.startX = fromX;
    this.startY = fromY;
  }

  void draw() {
    noFill();
    stroke(255, 50);
    line(startX, startY, endX, endY);
    ellipse(endX, endY, radius, radius);
  }

}

class Path {

  LinkedList checkpoints;
  float startX, startY;
  float endX, endY;
  
  Path(float startX, float startY) {
    checkpoints = new LinkedList();
    this.startX = startX;
    this.startY = startY;
  }

  void addCheckPoint(float heading, float distance, float radius) {
    CheckPoint cp = new CheckPoint(heading, distance, radius);
    if(checkpoints.size() == 0) {
      cp.setAbsolutePosition(startX, startY);
    } else {
      cp.setAbsolutePosition(endX, endY);
    }
    endX = cp.endX;
    endY = cp.endY;
    checkpoints.offer(cp);
    
  }
  
  void draw() {
    for(int i=0; i<checkpoints.size(); i++) {
      ((CheckPoint)checkpoints.get(i)).draw();
    }
  }
}






