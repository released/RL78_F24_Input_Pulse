# RL78_F24_Input_Pulse
 RL78_F24_Input_Pulse

udpate @ 2024/05/13

1. use RL78 F24 EVB to test input pulse

- use P12 (TAU1_0) , to generate 10K freq , and change duty by terminal , 

--press digit:a/A(increase duty) , d/D(decrease duty)

-- change duty with step : 0.1 %

- use P17 (TAU0_0) , to capture input pulse , to calculate freqency and duty

-- Set the TI00 pin input valid edge to “Both edge”.

2. print capture pulse log per 100 ms 

3. below is log and waveform


![image](https://github.com/released/RL78_F24_Input_Pulse/blob/main/log_duty_95.jpg)


![image](https://github.com/released/RL78_F24_Input_Pulse/blob/main/waveform_duty_95.jpg)


![image](https://github.com/released/RL78_F24_Input_Pulse/blob/main/log_duty_60.jpg)


![image](https://github.com/released/RL78_F24_Input_Pulse/blob/main/waveform_duty_60.jpg)

