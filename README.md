# Agrotech Lab 2024 
## Final Project
By: Ella Faruchi, Rotem David, Matar Tal, and Asif Charazi
### Project Description
For our project, we created a system that works with a home chicken coop, and alerts when there is an egg to be picked up from the coop.
The goal of the project was to make life a bit easier, by eliminating the need to check the coop for new eggs - sometimes a couple times a day - withount knowing if there is going to be an egg, or even multiple eggs. By using different sensors, we created a system that detects when there are eggs in the laying box, and can tell the person in the kitchen if and how many eggs are there currently. Because the coop we are working with has three chickens, our system can detect up to three eggs, but could be easily tweaked to detect as many as neccessary.

### The Sensors
**Weight-** the weight sits in the chickens' laying box, covered by cardboard and some hay. When it detects any weight above 2 Kg, aka a chicken, it ignores it. After the chicken has gotten out and only the egg remains, the detected weight is 40-60 grams and a signal is sent to the screen in the kitchen to announce that there is an egg in the coop. If the detected weight is 80-120 grams or 120-180 grams, the screen will display that there are two or three eggs accordingly.  
**Motion Sensor-** sits at the opening of the laying box and recognizes when a chicken enters the box. Sends a signal to the screen inside the house to write a message that there is currently a chicken in the laying box.  
**Distance Sensor-** sits at the back of the laying box (facing forward) and collects information about the chickens approaching the laying box.  
**Infra-red Thermometer-** sits inside the laying box and senses through heat when the chickens are inside the laying box.  

Each weight has a slightly different job, the weight being the one we are mainly counting on to give us information about weather there is an egg to be collected or not. The other sensors' job is to collect information, which after some time we hope that we will be able to calibrate with the information that we are getting from the weight for future use of the other sensors independently or as a backup for one another. For example, we hope to be able to calibrate the amount of time a chicken sits in the laying box to the weight informing about an egg being layed. In addition, the sensors can help us learn about the daily habits of the chickens by giving us a database of general information about them. We could use this database to track things such as changes in egg laying habits following a diet change.  

