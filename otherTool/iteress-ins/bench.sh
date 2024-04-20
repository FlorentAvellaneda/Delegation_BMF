
#!/usr/bin/env bash


echo -e "file\tk\tErrors\tTime (sec)"
/usr/bin/time -f "%e" ./execut.sh $((50*1)) audio.dat
/usr/bin/time -f "%e" ./execut.sh $((50*2)) audio.dat
/usr/bin/time -f "%e" ./execut.sh $((50*3)) audio.dat


/usr/bin/time -f "%e" ./execut.sh $((39*1)) autism.dat
/usr/bin/time -f "%e" ./execut.sh $((39*2)) autism.dat
/usr/bin/time -f "%e" ./execut.sh $((39*3)) autism.dat


/usr/bin/time -f "%e" ./execut.sh $((6*1)) balance.dat
/usr/bin/time -f "%e" ./execut.sh $((6*2)) balance.dat
/usr/bin/time -f "%e" ./execut.sh $((6*3)) balance.dat


/usr/bin/time -f "%e" ./execut.sh $((23*1)) breast.dat
/usr/bin/time -f "%e" ./execut.sh $((23*2)) breast.dat
/usr/bin/time -f "%e" ./execut.sh $((23*3)) breast.dat


/usr/bin/time -f "%e" ./execut.sh $((6*1)) car_dataset.dat
/usr/bin/time -f "%e" ./execut.sh $((6*2)) car_dataset.dat
/usr/bin/time -f "%e" ./execut.sh $((6*3)) car_dataset.dat


/usr/bin/time -f "%e" ./execut.sh $((10*1)) chess.dat
/usr/bin/time -f "%e" ./execut.sh $((10*2)) chess.dat
/usr/bin/time -f "%e" ./execut.sh $((10*3)) chess.dat


/usr/bin/time -f "%e" ./execut.sh $((18*1)) cmc_dataset.dat
/usr/bin/time -f "%e" ./execut.sh $((18*2)) cmc_dataset.dat
/usr/bin/time -f "%e" ./execut.sh $((18*3)) cmc_dataset.dat


/usr/bin/time -f "%e" ./execut.sh $((49*1)) dermato.dat
/usr/bin/time -f "%e" ./execut.sh $((49*2)) dermato.dat
/usr/bin/time -f "%e" ./execut.sh $((49*3)) dermato.dat



/usr/bin/time -f "%e" ./execut.sh $((11*1)) flare.dat
/usr/bin/time -f "%e" ./execut.sh $((11*2)) flare.dat
/usr/bin/time -f "%e" ./execut.sh $((11*3)) flare.dat


/usr/bin/time -f "%e" ./execut.sh $((68*1)) heart.dat
/usr/bin/time -f "%e" ./execut.sh $((68*2)) heart.dat
/usr/bin/time -f "%e" ./execut.sh $((68*3)) heart.dat


/usr/bin/time -f "%e" ./execut.sh $((39*1)) hepatits.dat
/usr/bin/time -f "%e" ./execut.sh $((39*2)) hepatits.dat
/usr/bin/time -f "%e" ./execut.sh $((39*3)) hepatits.dat


/usr/bin/time -f "%e" ./execut.sh $((32*1)) iris.dat
/usr/bin/time -f "%e" ./execut.sh $((32*2)) iris.dat
/usr/bin/time -f "%e" ./execut.sh $((32*3)) iris.dat


/usr/bin/time -f "%e" ./execut.sh $((8*1)) lungCancer.dat
/usr/bin/time -f "%e" ./execut.sh $((8*2)) lungCancer.dat
/usr/bin/time -f "%e" ./execut.sh $((8*3)) lungCancer.dat


/usr/bin/time -f "%e" ./execut.sh $((14*1)) lymph.dat
/usr/bin/time -f "%e" ./execut.sh $((14*2)) lymph.dat
/usr/bin/time -f "%e" ./execut.sh $((14*3)) lymph.dat


/usr/bin/time -f "%e" ./execut.sh $((28*1)) mushroomFULL.dat
/usr/bin/time -f "%e" ./execut.sh $((28*2)) mushroomFULL.dat
/usr/bin/time -f "%e" ./execut.sh $((28*3)) mushroomFULL.dat


/usr/bin/time -f "%e" ./execut.sh $((8*1)) nursery.dat
/usr/bin/time -f "%e" ./execut.sh $((8*2)) nursery.dat
/usr/bin/time -f "%e" ./execut.sh $((8*3)) nursery.dat


/usr/bin/time -f "%e" ./execut.sh $((7*1)) phishing.dat
/usr/bin/time -f "%e" ./execut.sh $((7*2)) phishing.dat
/usr/bin/time -f "%e" ./execut.sh $((7*3)) phishing.dat


/usr/bin/time -f "%e" ./execut.sh $((25*1)) soybean.dat
/usr/bin/time -f "%e" ./execut.sh $((25*2)) soybean.dat
/usr/bin/time -f "%e" ./execut.sh $((25*3)) soybean.dat


/usr/bin/time -f "%e" ./execut.sh $((44*1)) student.dat
/usr/bin/time -f "%e" ./execut.sh $((44*2)) student.dat
/usr/bin/time -f "%e" ./execut.sh $((44*3)) student.dat


/usr/bin/time -f "%e" ./execut.sh $((85*1)) thoraric.dat
/usr/bin/time -f "%e" ./execut.sh $((85*2)) thoraric.dat
/usr/bin/time -f "%e" ./execut.sh $((85*3)) thoraric.dat


/usr/bin/time -f "%e" ./execut.sh $((7*1)) tictactoe.dat
/usr/bin/time -f "%e" ./execut.sh $((7*2)) tictactoe.dat
/usr/bin/time -f "%e" ./execut.sh $((7*3)) tictactoe.dat


/usr/bin/time -f "%e" ./execut.sh $((11*1)) tumor.dat
/usr/bin/time -f "%e" ./execut.sh $((11*2)) tumor.dat
/usr/bin/time -f "%e" ./execut.sh $((11*3)) tumor.dat


/usr/bin/time -f "%e" ./execut.sh $((4*1)) votes.dat
/usr/bin/time -f "%e" ./execut.sh $((4*2)) votes.dat
/usr/bin/time -f "%e" ./execut.sh $((4*3)) votes.dat


/usr/bin/time -f "%e" ./execut.sh $((45*1)) wine.dat
/usr/bin/time -f "%e" ./execut.sh $((45*2)) wine.dat
/usr/bin/time -f "%e" ./execut.sh $((45*3)) wine.dat


/usr/bin/time -f "%e" ./execut.sh $((7*1)) zoo.dat
/usr/bin/time -f "%e" ./execut.sh $((7*2)) zoo.dat
/usr/bin/time -f "%e" ./execut.sh $((7*3)) zoo.dat



