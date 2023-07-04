# ROBDD Generation 
### install graphviz
```
sudo yum install graphviz
```

### convert pla file to dot file
```
g++ ROBDD.cpp -o ROBDD
./ROBDD [plafile.pla] [obdd.dot] [robdd.dot]
```
ex: \
./ROBDD 4var.pla 4varOBDD.dot 4varROBDD.dot

### convert to png  file
```
dot -T png [obdd.dot] > [obdd.png]
```
ex: \
dot -T png 4varOBDD.dot > 4varOBDD.png\
dot -T png 4varROBDD.dot > 4varROBDD.png\
dot -T png 5varOBDD.dot > 5varOBDD.png\
dot -T png 5varROBDD.dot > 5varROBDD.png\
