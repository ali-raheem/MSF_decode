# MSF_decode
Arduino decoder for the National Physical Laboratory MSF radio time code


## Files
- MSFdecoder - interrupt based decoder
- MSF_decode - non-interrupt based decoder

[Overview](https://www.npl.co.uk/msf-signal) and [wiki article](https://en.wikipedia.org/wiki/Time_from_NPL_(MSF))

[Technical information](https://www.npl.co.uk/products-services/time-frequency/msf-radio-time-signal/msf_time_date_code)


![Breadboarded](https://github.com/ali-raheem/MSF_decode/assets/355742/f04e78c3-c0ac-4550-add4-b6d3128c5884)


![Serial output](https://github.com/ali-raheem/MSF_decode/assets/355742/f1ab7966-4055-4efc-9480-f119e6c31cc2)


## Todo
1. Rewrite with bitfields not whole bytes to hold bits..
2. Clean up code
3. Integrate with [gameoflife](https://github.com/ali-raheem/game_of_life)
4. ~~Rewrite to be interrupt driven~~

## Utils
Use MSF_tune to adjust the antenna if needed(check the plotter `ctrl+shift+L`. A good enough response is shown in the plotter below. the minute marker is the meeting in the middle at 500ms.
![image](https://github.com/ali-raheem/MSF_decode/assets/355742/d8541e46-f563-4cc1-bbde-e0fc2c467df1)
