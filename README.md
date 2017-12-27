
1. normal data channel 與 priority data channel要可以同時傳送資料(測試時從client輸入兩筆資料)。

2. server要能服務多個client，且要能顯示每個client的兩個port(priority port 與 client port)，server那邊有自己個用戶資料，他會知道誰是他的用戶，請用cookie去判斷(我沒用就是了，使用linkedlist)。

3.要能拒絕惡意連線的client(不是你的用戶) 。
