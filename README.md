# rasbperey-dart-HOCH
树莓派-甲醛检测
1.组件
raspberryp
dart 甲醛 sensor
若干杜邦线

2.大概实现过程
整个过程比较简单
在树莓派上跑一个后台进程,采集传感器的数据然后写入到数据库,然后理由web展示出来

3.注意事项
a.dart 传感器是串口通信,我这边利用gpio来实现串口,确认一下配置
# ls -l /dev/serial*
/dev/serial0 -> ttyAMA0
/dev/serial1 -> ttyS0
默认情况serial0是蓝牙设备,通过修改配置,把蓝牙映射的串口改成serial1,具体配置可以参考rasbperey_config的配置文件.
b.传感器和gpio接线示意图
+++++++++++++++++++++++++++++++++++++++++++++++++++++
      |TXD      RXD  |
GPIO  |              |dart hoch sensor
      |RXD      TXD  |    

++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                               
         
                                             
具体GPIO示意图可以到网上查看.
4.安装
a.gcc -lmysqlclient dart.c -o hoch
b.cp -a web/* /var/www/html
c.修改/etc/php/7.0/apache2/php.ini中的short_open_tag为on


