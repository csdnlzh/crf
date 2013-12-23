time ./crf --train ../../data/sighan/train/msr_training.utf8.fea --it 100 --thread 20 --learn_rate 0.05  --debug 0
./crf --test ../../data/sighan/test/msr_test.utf8.fea  --model model.txt
python tag2word.py out_tag msr_predict.txt
cp ./msr_predict.txt ../../data/sighan
cd ../../data/sighan
./score ./ msr_test_gold.utf8 msr_predict.txt > msr_log
