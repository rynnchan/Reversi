#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef enum{
    EMPTY, //空白 0
    BRACK, //黒 先攻 1
    WHITE, //白 後攻 2
    PUT, // 置ける場所 3
    OUT // 場外 4
} state;

#define BOARD_SIZE 8 //ボードの幅
#define DEPTH 5 //探索の深さ

//ボードの評価値
#define EV0   -1
#define EV1   30
#define EV2   -12
#define EV3   0
#define EV4   -1
#define EV5   -15
#define EV6   -3
#define EV7   -3
#define EV8   0
#define EV9   -1

#define prscf(s,a) (printf("%s",s),scanf("%d",a)) //printfとscanfを同時に行う関数形式マクロ

/**
 * ボードを管理する構造体
*/
typedef struct {
    int board[BOARD_SIZE+2][BOARD_SIZE+2]; // ボードの情報
    int evalution[BOARD_SIZE+2][BOARD_SIZE+2]; // ボードの評価
    int remine; // 残りの空き数
    int num[3]; // 今の枚数
    char *mark[4]; //オセロを表示するマーク
}Board;

/**
 * プレイヤーの管理をする構造体
*/
typedef struct{
    char *name[10]; //名前
    int attackNum; //先攻後攻
    int moves; //手数
}Player;

/**
 * ボードを表示する関数
 * ボードの他に、黒と白のそれぞれの枚数と残り空き数を表示する
 */
void print(Board *b){
    printf("\\|");
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        printf("%d|",i);
    }
    printf("\n-+-+-+-+-+-+-+-+-+\n");
    for(i=1;i<=BOARD_SIZE;i++){
        printf("%d|",i);
        for(j=1;j<=BOARD_SIZE;j++){
            printf("%s|",b->mark[b->board[i][j]]);
        }
        printf("\n");
    }
    //黒と白の今現在の数と、残りの数を表示する
    printf("%s %d v.s %s %d (残り：%d)\n",
        b->mark[BRACK],b->num[BRACK],b->mark[WHITE],b->num[WHITE],b->remine);
}

/**
 * 置ける場所があるか無いかを確認し、その手数を返す関数
 *  8方向全てを確認する方法
*/
int possibleAll(int me,Board *b,Player *p){
    p->moves=0;
    int i;
    int j;
    int k;
    //前のPUTを全てEMPTYに戻す
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b->board[i][j]==PUT)
                b->board[i][j]=EMPTY;
        }
    }    
    int d[8][2]={{1,1},{1,0},{1,-1},{0,1},{0,-1},{-1,1},{-1,0},{-1,-1}}; //８方向
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b->board[i][j]==EMPTY){
                for(k=0;k<8;k++){
                    int r=i;
                    int c=j;
                    while(b->board[i+d[k][0]][j+d[k][1]]!=me){
                        r+=d[k][0];
                        c+=d[k][1];
                        if(b->board[r][c]==me){ //置けるなら
                            p->moves++;
                            b->board[i][j]=PUT;
                            break;
                        }
                        else if(b->board[r][c]==EMPTY||b->board[r][c]==PUT||b->board[r][c]==OUT){ //置ないことが決まったら
                            break;
                        }
                    }
                    if(b->board[i][j]==PUT)
                        break;
                }
            }
        }
    }
    return p->moves;
}

/**
 * 一つの場所について、置けるか否かを返す関数
 * 8方向全てを確認する方法
 * 置けるならそこに置き、挟んだ場所をひっくり返す
*/
int possiblePart(int me,Board *b,int row,int col){
    if(b->board[row][col]==PUT){
        int d[8][2]={{1,1},{1,0},{1,-1},{0,1},{0,-1},{-1,1},{-1,0},{-1,-1}};
        int enemy=me%2+1;
        int i;
        for(i=0;i<8;i++){
            int r=row;
            int c=col;
            while(b->board[row+d[i][0]][col+d[i][1]]!=me){
                r+=d[i][0];
                c+=d[i][1];        
                if(b->board[r][c]==EMPTY||b->board[r][c]==PUT||b->board[r][c]==OUT){
                    break;
                }else if(b->board[r][c]==me){ //挟む対象が確定したら
                    int s;
                    int t;
                    //挟んだ場所をひっくり返す
                    for(s=row+d[i][0],t=col+d[i][1];!(s==r&&t==c);s+=d[i][0],t+=d[i][1]){
                        b->board[s][t]=me;
                        b->num[me]++;
                        b->num[enemy]--;
                    }
                break;
                }        
            }
        }
        b->board[row][col]=me;
        b->num[me]++;
        b->remine--;        
        return 1;
    }
    else{
        return 0;
    }
}

/**
 * どこに置くかを決める関数
 * ボード上ならpossiblePart関数に入り、
 * 置なかったら再び置く場所を決める
*/
void choice(int me,Board *b){
    int row,col;
    while(1){
        prscf("どこに打つ？\n行：",&row);
        prscf("列：",&col);
        if(row>=1&&row<=8&&col>=1&&col<=8&&possiblePart(me,b,row,col))
            break;
        printf("one more time\n");
    }
}

/**
 * 引数のボードの評価を返す関数
 * 評価＝自分の石の評価値ー相手の石の評価値
*/
int evalution(int me,Board b){
    int total=0;
    int enemy=(me%2)+1;
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b.board[i][j]==me)
                total+=b.evalution[i][j];
            else if(b.board[i][j]==enemy)
                total-=b.evalution[i][j];
        }
    }
    return total;
}

/**
 * row,colの位置に置いた後のボードの状態を返す関数
*/
Board upset(int me,Board b,int row,int col){
    Board *bb=&b;
    int d[8][2]={{1,1},{1,0},{1,-1},{0,1},{0,-1},{-1,1},{-1,0},{-1,-1}};
    int enemy=me%2+1;
    int i;
    for(i=0;i<8;i++){
        int r=row;
        int c=col;
        while(bb->board[row+d[i][0]][col+d[i][1]]!=me){
            r+=d[i][0];
            c+=d[i][1];        
            if(bb->board[r][c]==EMPTY||bb->board[r][c]==PUT||bb->board[r][c]==OUT){
                break;
            }else if(bb->board[r][c]==me){
                for(int s=row+d[i][0],t=col+d[i][1];!(s==r&&t==c);s+=d[i][0],t+=d[i][1]){
                    bb->board[s][t]=me;
                }
                break;
            }        
        }
    }
    bb->board[row][col]=me;
    return *bb;
}

/**
 * 評価値が最大となる場合の石の置く場所を返す関数
*/
int *evalutionMax(int me,Board b){
    float max=-INFINITY;
    float evaSave=-INFINITY;
    int *ans;
    int row=0,col=0;
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b.board[i][j]==PUT){
                Board bb=upset(me,b,i,j);
                evaSave=evalution(me,bb);
                if(evaSave>max){
                    max=evaSave;
                    row=i;
                    col=j;
                }
            }
        }
    }
    *ans=row;
    *(ans+1)=col;
    return ans;
}

/**
 * 現在のボード嬢の自分の石の数を返す関数
*/
int counter(int me,Board b){
    int count=0;
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b.board[i][j]==me)
                count++;
        }
    }
    return count;
}

/**
 * negamaxとαβ法を用いて、評価値が最良となる石の置き場所を返す関数
*/
int negamax(int me,Board b,Player p,int depth,float alpha,float beta,int count,int *ans){
    int pa=possibleAll(me,&b,&p);
    if(depth<=0||count>=2)
        return 2*pa+evalution(me,b);
    if(counter(me,b)==0)
        return 1000;
    int enemy=(me%2)+1;
    if(pa==0){
        return -negamax(enemy,b,p,depth-1,-beta,-alpha,count+1,ans); 
    }
    int save=0;
    float max=alpha;
    int row=0,col=0;
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b.board[i][j]==PUT){
                Board bb=upset(me,b,i,j);
                save=-negamax(enemy,bb,p,depth-1,-beta,-max,0,ans); 
                if(save>max){
                    max=save;
                    row=i;
                    col=j;
                }
                if(max>=beta)
                    break;
            }
        }
        if(max>=beta)
            break;
    }         
    *ans=row;
    *(ans+1)=col;

    return max;
}

/**
 * negamaxとαβ法を用いて終盤の完全読み切りをして、
 * 最終的な自分の石の数が最大になるような石の置く場所を返す関数
*/
int lastStage(int me,Board b,Player p,int depth,float alpha,float beta,int count,int *ans){
    int pa=possibleAll(me,&b,&p);
    if(depth<=0||count>=2)
        return counter(me,b);
    if(counter(me,b)==0)
        return 1000;
    int enemy=(me%2)+1;
    int save=0;
    float max=alpha;
    int row=0,col=0;
    if(pa==0)
        return -lastStage(enemy,b,p,depth,-beta,-alpha,count+1,ans++);
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(b.board[i][j]==PUT){
                Board bb=upset(me,b,i,j);
                save=-lastStage(enemy,bb,p,depth-1,-beta,-max,0,ans); 
                if(save>max){
                    max=save;
                    row=i;
                    col=j;
                }
                if(max>=beta)
                    break;
            }
        }
        if(max>=beta)
            break;
    }    
    *ans=row;
    *(ans+1)=col;

    return max;
}

/**
 * CPU0の実装をした関数
 * ボードの1/1から順に探索をし、置ける場所があればそこに置く
*/
void cpu0(int me,Board *b){
    int sign=0;
    int i;
    int j;
    for(i=1;i<=BOARD_SIZE;i++){
        for(j=1;j<=BOARD_SIZE;j++){
            if(possiblePart(me,b,i,j)){
                printf("CPUは(%d,%d)に置きました。\n",i,j);
                sign=1;
                break;
            }
        }
        if(sign)
            break;
    }
}

/**
 * CPU1の実装をした関数
 * 今現在のボードでもっとも評価値が高くなるように石を置く
*/
void cpu1(int me,Board *b){
    int *a;
    a=evalutionMax(me,*b);
    if(possiblePart(me,b,a[0],a[1]))
        printf("CPUは(%d,%d)に置きました。\n",a[0],a[1]);
}

/**
 * AIの実装をした関数
 * negaαで探索をし、終盤になると完全読み切りをする
*/
void ai(int me,Board *b,Player p,int depth){
    int *a;
    a=(int *)malloc(2*sizeof(int));
    float alpha=-INFINITY;
    float beta=INFINITY;
    int max;
    int (*func[])(int,Board,Player,int,float,float,int,int *)={lastStage,negamax};
    if(b->remine<=12){
        max=(*func[0])(me,*b,p,b->remine,alpha,beta,0,a);
    }else{
        max=(*func[1])(me,*b,p,depth,alpha,beta,0,a);
    }

    if(possiblePart(me,b,a[0],a[1])){
        printf("AIは(%d,%d)に置きました。\n",a[0],a[1]);
    }
}

/**
 * メイン関数
*/
int main(void){
    Board b={
        {//ボードの情報
        {OUT,OUT,OUT,OUT,OUT,OUT,OUT,OUT,OUT,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,WHITE,BRACK,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,BRACK,WHITE,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,OUT},
        {OUT,OUT,OUT,OUT,OUT,OUT,OUT,OUT,OUT,OUT},
        },
        {//ボードの評価
        {0,0,0,0,0,0,0,0,0,0},
        {0,EV1,EV2,EV3,EV4,EV4,EV3,EV2,EV1,0},
        {0,EV2,EV5,EV6,EV7,EV7,EV6,EV5,EV2,0},
        {0,EV3,EV6,EV8,EV9,EV9,EV8,EV6,EV3,0},
        {0,EV4,EV7,EV9,EV0,EV0,EV9,EV7,EV4,0},
        {0,EV4,EV7,EV9,EV0,EV0,EV9,EV7,EV4,0},
        {0,EV3,EV6,EV8,EV9,EV9,EV8,EV6,EV3,0},
        {0,EV2,EV5,EV6,EV7,EV7,EV6,EV5,EV2,0},
        {0,EV1,EV2,EV3,EV4,EV4,EV3,EV2,EV1,0},
        {0,0,0,0,0,0,0,0,0,0},
        },
        BOARD_SIZE*BOARD_SIZE-4, //残りの空き数
        {0,2,2}, //黒と白の数（黒と白を1,2番にした方が都合が良いため0番を空けた）
        {" ","●","○","◎"}, //1.黑のマーク,2.白のマーク,3.着手可能のマーク

    };

    int enemyNum;
    while(1){ //対戦相手の選択
        prscf("相手を選んでね[1.人,2.CPU(さいじゃく),3.CPU(やさしい),4.AI(かんたん)]:",&enemyNum);
        if(enemyNum>=1&&enemyNum<=4)
            break;
        else
            enemyNum=0;
    }
    int senko;
    while(1){ //順番の選択
        prscf("先攻後攻を選んでね[1.先攻(黒),2.後攻(白)]:",&senko);
        if(senko==BRACK||senko==WHITE)
            break;
        else
            senko=0;
    }

    Player player[3]={
        {NULL,NULL,NULL},
        {{"あなた"},senko,0},
        {{"相手"},senko%2+1,0}
    };

    printf("Game Start!!\n");
    
    int a=senko;
    int count=0; //両方が置けなくなった用
    while(1){ //行動
        if(b.remine==0){ //空いている場所が無いなら
            print(&b);
            break;
        }
        if(possibleAll(player[a].attackNum,&b,&player[a])){ //置ける場所があるなら
            print(&b);
            printf("%s(%s)\n",*player[a].name,b.mark[player[a].attackNum]);
            printf("着手可能数：%d\n",player[a].moves);
            if(a==1||enemyNum==1) //自分または「人」
                choice(player[a].attackNum,&b);
            else if(enemyNum==2) //「CPU0」
                cpu0(player[a].attackNum,&b);
            else if(enemyNum==3) //「CPU1」
                cpu1(player[a].attackNum,&b);
            else if(enemyNum==4) //「AI」
                ai(player[a].attackNum,&b,player[a],DEPTH);
            count=0;
        }else{ //置ける場所が無いなら
            printf("%sは置ける場所が無いため、パスします。\n",*player[a].name);
            count++;
            if(count==2)
                break;
        }
        a=(a%2)+1; //1or2
    }
    //勝負が決まった後の表示
    printf("%s %d v.s %s %dで",b.mark[BRACK],b.num[BRACK],b.mark[WHITE],b.num[WHITE]);
    if(b.num[BRACK]>b.num[WHITE])
        printf("%s( %s )の勝ち！\n",*player[senko].name,b.mark[BRACK]);
    else if(b.num[BRACK]<b.num[WHITE])
        printf("%s( %s )の勝ち！\n",*player[(senko%2)+1].name,b.mark[WHITE]);
    else
        printf("引き分け！\n");

    //ファイルに勝敗と日時を記録する
    char *issue[5];
    if(b.num[player[1].attackNum]>b.num[player[2].attackNum])
        *issue=" 勝ち ";
    else if(b.num[player[1].attackNum]<b.num[player[2].attackNum])
        *issue=" 負け ";
    else
        *issue="引き分け";
    FILE *fp;
    if((fp=fopen("main.txt","a"))==NULL)
        printf("ファイルをオープンできませんでした。\n");
    else{
        time_t t;
        struct tm *local;
        time(&t);
        local=localtime(&t);
        fprintf(fp,"%s%d v.s %s%d   %s  %d年%d月%d日%d時%d分\n",
        b.mark[BRACK],b.num[BRACK],b.mark[WHITE],b.num[WHITE],*issue,
        local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min);
        fclose(fp);
    }

    return 0;
}
