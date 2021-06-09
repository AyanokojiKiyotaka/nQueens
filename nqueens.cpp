#pragma GCC optimize(2)

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <iostream>

#define re register
//diagonal_plus(x):第x列棋子所处于的一条对角线上的棋子数
//#define diagonal_plus(x) diagonal_plus[x+map[x]]
//diagonal_minus(x):第x列棋子所处于的另外一条对角线上的棋子数
//#define diagonal_minus(x) diagonal_minus[x-map[x]+len-1]

using namespace std;

int len, diaglen;      //len:棋盘大小   diaglen:同一个方向的对角线数目
int *map;        //存储棋子位置，表示每一列的棋子所在的行数
int *diagonal_plus, *diagonal_minus; //对角线上的棋子数
int *lines;        //用于排序，存储列的编号，然后按照其上棋子所在对角线的棋子数进行排序
int *diagonal;       //diagonal[i]表示第1列棋子所在对角线上的棋子数总和

inline int *diagonal_plus_(int x) //基本同被注释掉的#define diagonal_plus(x)
{
 return diagonal_plus + x + map[x];
}
inline int *diagonal_minus_(int x) //同上
{
 return diagonal_minus + x - map[x] + len - 1;
}

inline bool cmp(int a, int b) //用于std::sort排序
{
 return diagonal[a] > diagonal[b];
}

inline int colls(int x) //对角线上碰撞数，x为棋子数，如果只有一个棋子或无棋子，则返回0，否则返回x-1
{
 return x > 1 ? x - 1 : 0;
}

inline int initEvaluate() //初始evaluate  o(n)
{
 memset(diagonal_minus, 0, sizeof(int) * diaglen);
 memset(diagonal_plus, 0, sizeof(int) * diaglen); //初始化
 for (re int i = 0; i < len; ++i)
 {
  ++*diagonal_plus_(i);
  ++*diagonal_minus_(i); //统计对角线棋子数
 }
 re int ans = 0;
 for (re int i = 0; i < diaglen; ++i)
 {
  ans += colls(diagonal_minus[i]) + colls(diagonal_plus[i]); //统计答案
 }
 return ans;
}

inline void initdiag() //计算diagonal,用于排序
{
 for (re int i = 0; i < len; ++i)
 {
  diagonal[i] = *diagonal_minus_(i) + *diagonal_plus_(i);
 }
}

inline void restart() //随机交换len/4次，完全打乱
{
 for (re int i = 0; i < len / 4; ++i)
  swap(*(map + (rand() % len)), *(map + (rand() % len)));
 //printf("restart\n");
}

inline int swapWithEvaluate(int x, int y) //交换棋子，同时以时间复杂度o(1)计算value的变化
{
 re int delta = 0;

 if (--*diagonal_plus_(x) > 0)
  --delta;
 if (--*diagonal_minus_(x) > 0)
  --delta;
 if (--*diagonal_plus_(y) > 0)
  --delta;
 if (--*diagonal_minus_(y) > 0)
  --delta; //模拟把要交换的棋子从棋盘上取出

 swap(map[x], map[y]);
 //map[x] ^= map[y] ^= map[x] ^= map[y];

 if (++*diagonal_plus_(x) > 1)
  ++delta;
 if (++*diagonal_minus_(x) > 1)
  ++delta;
 if (++*diagonal_plus_(y) > 1)
  ++delta;
 if (++*diagonal_minus_(y) > 1)
  ++delta; //模拟把要交换的棋子放回棋盘

 return delta;
}

int main()
{
 clock_t start, end;
 start = clock(); //计时
 //freopen("out.txt", "w", stdout);
 srand((unsigned)time(0)); //随机数种子
 len = 1000000;
 diaglen = len * 2 - 1;
 map = new int[len];
 diagonal_plus = new int[diaglen];
 diagonal_minus = new int[diaglen];
 diagonal = new int[len];
 lines = new int[len]; //初始化各个数组

 memset(map, 0, sizeof(int) * len);
 for (re int index = 0; index < len; ++index)
 {
  lines[index] = map[index] = index;
 } //初始化lines,和map

 re int value, restTime = 0;
 /*do{
  restart();
  if (++restTime > 20)
  {
   value = initEvaluate();
   break;
  }
 }while((value=initEvaluate())>7*len/10);   //value直到降低到6/11*len，restart初始解*/
 restart();
 value = initEvaluate();
 re int finalAns = 0;       //最终value下降到finalAns时得到解
 re int delta = 0;        //value的变化值
 re int noAnsTimes = 0;       //找不到无法梯度下降的邻居的次数
 re int reduceTimesUntilSort = 30000;    //每梯度下降reduceTimesUntilSort次，sort一次lines
 re int reduceTime = reduceTimesUntilSort; //已经下降的次数
 while (value > finalAns)      //开始爬山主循环
 {
  ++reduceTime;
  if(reduceTime>reduceTimesUntilSort || (value<=40000&&reduceTime>100))  //已经下降了reduceTimesUntilSort次了，进行一次lines排序
  {
   initdiag();
   sort(lines,lines+len,cmp);
   reduceTime=0;
  }
  for (re int x = 0; x < len; ++x) //从碰撞数最高点棋子开始，选择交换的第一个棋子i=lines[x]
  {
   re int i = lines[x];
   if (*diagonal_minus_(i) <= 1 && *diagonal_plus_(i) <= 1)
    continue;
   for (re int y = x + 1; y < len; ++y) //选择交换的第二个棋子j=lines[y]
   {
    re int j = lines[y];
    //if (*diagonal_minus_(j) <= 1 && *diagonal_plus_(j) <= 1 && * diagonal_minus_(i) <= 1 && *diagonal_plus_(i) <= 1) continue;
    if (*diagonal_minus_(j) <= 1 && *diagonal_plus_(j) <= 1)
     continue;

    delta = swapWithEvaluate(i, j); //尝试交换

    if (delta < 0) //能梯度下降
    {
     break; //选取该解，退出历遍邻居循环
    }

    swapWithEvaluate(i, j); //无法梯度下降，进行回溯
   }
   if (delta < 0) //找到能梯度下降的解，退出历遍邻居循环
   {
    break;
   }
  }
  if (delta < 0) //找到能梯度下降的解
  {
   value += delta;       //记录value变化
   printf("%d\n", value - finalAns); //输出距离最终结果的距离
   continue;        //进行下一次梯度下降
  }
  if (noAnsTimes > len / 100) //未找到能梯度下降的邻居，并且次数超过了允许次数，进行restart
  {
   noAnsTimes = 0;
   /*do{
    restart();
   }while((value=initEvaluate())>7*len/10);*/
   restart();
   value = initEvaluate();
   //printf("%d\n",value-finalAns);
   continue;
  }
  ++noAnsTimes;       //未找到能梯度下降的邻居,记录失败次数+1
  for (re int i = 0; i < 3; ++i) //随机交换3次
  {
   re int x, y;
   x = (rand() << 15 + rand()) % len;
   while (x < 0)
    x += len;
   for (y = (rand() << 15 + rand()) % len;; y = (rand() << 15 + rand()) % len)
   {
    while (y < 0)
     y += len;
    if (x != y)
     break;
   }
   value += swapWithEvaluate(x, y);
  }
  //srand((unsigned)time(0));
  printf("%d\n", value - finalAns);
 }
 end = clock();
 //printf("chk value: %d\n",initEvaluate()-finalAns);  //检查结果是否正确用
 //for(re int i=0;i<len;++i) printf("%d ",map[i]);       //输出结果
 std::cout << std::endl
     << "len=" << len << std::endl
     << "Time=" << (double)(end - start) / CLOCKS_PER_SEC << 's' << std::endl;
 system("pause");
}
