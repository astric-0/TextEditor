#include<iostream>
#include<fstream>
#include<vector>
#include<ncurses.h>
#include<dirent.h>
using namespace std;

struct List{
    string title;
    string data;
    int session;
    List* link;
};

struct Do{
    string data;
    Do* link;
};

class Stack{
    Do* top;

    public:
    Stack(){
        this->top=NULL;
    }

    Do* getTop(){
        return this->top;
    }

    void push(const string& text){
        Do* temp = new Do();                
        temp->data=text, temp->link=NULL;
        (this->top==NULL) ? this->top = temp : temp->link=this->top, this->top=temp;                        
    }

    string pop(){
        Do* temp=this->top;
        string text=this->top->data;
        this->top=this->top->link;
        delete temp;

        return text;
    }

    void clean(){
        Do* ptr=this->top;
        while(ptr!=NULL){
            Do* temp= new Do();
            temp = ptr;
            ptr=ptr->link;
            delete temp;
        }
        this->top=new Do();
        this->top=NULL;
    }
};

class Menu{
    int x,y;
    int sx_in, sy_in;    
    WINDOW *locs, *menubox, *alert, *in, *cmd, *curr;
    List *head, *end;
    string menu[2][6];    
    int total_sessions;

    void _writer(int& curr_y, int& curr_x, string data){
        int i=0;
        while(data[i]!='\0'){    

            if(curr_x==getmaxx(in)-2){
                curr_x=1, curr_y++;
                this->_printnum(curr_y);
            }              

            if(data[i]=='\n'){
                curr_y++, curr_x=1;
                this->_printnum(curr_y);               
                wmove(this->in, curr_y, curr_x);
            }

            else
                wmove(this->in, curr_y, ++curr_x), waddch(in,data[i]);
            i++;
        }
    }

    vector<string> _print_sessions(int& curr_y){        
/*
        wattron(this->in, COLOR_PAIR(3));
        mvwprintw(this->in, 0, 2, "%s", ("AVAILABE SESSIONS : "+to_string(this->total_sessions)).c_str());
        wattroff(this->in, COLOR_PAIR(3));
*/
        DIR *dr;        
        dr = opendir("./docs");
        
        vector<string> filenames;
        int s_no=1;        
        if(dr){
            struct dirent *en;
            while((en=readdir(dr))!=NULL){
                wattron(this->in, COLOR_PAIR(3));
                mvwprintw(this->in, curr_y, 2, "%s", to_string(s_no++).c_str());
                wattroff(this->in, COLOR_PAIR(3));

                wattron(this->in, COLOR_PAIR(4));
                mvwprintw(this->in, curr_y++, 5, "%s", en->d_name);            
                wattroff(this->in, COLOR_PAIR(4));

                filenames.push_back(en->d_name);
            }
        }     

        return filenames;   
    }

    void _printnum(int curr_y){
        wattron(this->in, COLOR_PAIR(3));
        mvwprintw(this->in,curr_y,0, "%s", to_string(curr_y).c_str());
        wattroff(this->in, COLOR_PAIR(3));
    }

    public:
    //
    int session_no;
    Menu(){        
        string a[2][6] = {
            { "[ 0. EXIT (q) ]", "[ 1. PRINT (p) ]", "[ 2. INSERT (w) ]", "[ 3. DELETE (d) ]", "[ 4. EDIT (e) ]", "[ 5. SEARCH (s) ]" },
            { "[ ^x EXIT ]", "[ ^o SAVE ]", "[ ^u UNDO ]"}
        };

        int rows = sizeof(a)/sizeof(a[0]);        
        for(int i=0;i<rows;i++){
            int cols = sizeof(a[i])/sizeof(a[0][0]);
            for(int j=0;j<cols;j++){
                this->menu[i][j]=a[i][j];
            }
        }

        this->session_no=1;
        this->total_sessions=0;
        this->head=NULL, this->end=NULL;

        getmaxyx(stdscr, this->y, this->x);                
        this->sy_in=4,this->sx_in=0;

        start_color();
        // foreground, background
        init_pair(1, COLOR_RED, COLOR_BLACK);    
        init_pair(2, COLOR_WHITE, COLOR_GREEN);
        init_pair(3, COLOR_RED, COLOR_WHITE);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_WHITE, COLOR_CYAN);
        init_pair(6, COLOR_GREEN, COLOR_BLACK);

        this->curr=NULL;

        /* (lines, rows, start_y, start_x) */        
        this->menubox = newwin(3, 0, 0, 0);
        box(this->menubox,0,0);
        wrefresh(this->menubox);

        this->locs = newwin(2, 5, y-2, x-5);    
        wrefresh(this->locs);

        this->alert = newwin(1, 50, y-2, 3);        
        wrefresh(this->alert);        

        //make scrollable
        this->in = newwin(y-6, 0, this->sy_in, this->sx_in);
        wrefresh(this->in);

        this->cmd = newwin(1, 10, 2, 2);
        wrefresh(this->cmd);

        refresh();
    }

    void makealert(string msg){
        wclear(this->alert);
        
        wattron(this->alert, COLOR_PAIR(4));
        wprintw(this->alert, "%s", msg.c_str());
        wattroff(this->alert, COLOR_PAIR(4));
        
        wrefresh(this->alert);
    }

    void options(int index){
        wclear(this->menubox);                
        int size = sizeof(this->menu[index])/sizeof(this->menu[0][0]);
        wmove(this->menubox,0,2);
        for(int i=0;i<size;i++){                        
            wattron(this->menubox, COLOR_PAIR(2)), wattron(this->menubox, A_BOLD);            
            wprintw(this->menubox, "%s", this->menu[index][i].c_str());
            wattroff(this->menubox, A_BOLD), wattroff(this->menubox, COLOR_PAIR(2));
            wprintw(this->menubox, "%s","    ");
        }
        wrefresh(this->menubox);  
    }

    int choice(){        
        wclear(this->cmd);
        wattron(this->cmd, COLOR_PAIR(1)), wattron(this->cmd, A_BOLD);
        wprintw(this->cmd,"[CMD]:");        
        wattroff(this->cmd, A_BOLD), wattroff(this->cmd, COLOR_PAIR(1));   
        
        int ch=wgetch(this->cmd);        
        
        wrefresh(this->cmd);
        return ch;
    }    
    //
    void locwin(int curr_y, int curr_x){
        wclear(locs);
        string s = to_string(curr_y) + ":" + to_string(curr_x);
        wprintw(locs,"%s",s.c_str());
        wrefresh(locs);
    }
    //
    void save(string data, const char* title){                
        List* ptr=head;
        while(ptr!=NULL){
            if(ptr->session==this->session_no){
                ptr->data=data;
                return;
            }
            ptr=ptr->link;
        }

        List* msg = new List();
        msg->title= title;
        msg->session = this->session_no;
        msg->data = data;        
        msg->link = NULL;
        
        this->total_sessions++;
        (head==NULL) ? head=msg, end=msg : end->link=msg, end=msg;             
    }
    
    void print(){
        /*
        if(head==NULL){
            this->makealert("NOTHING TO PRINT (press any key)"), getchar();
            return;
        }                
        */
        wclear(this->in);
        int curr_y=2;
        vector<string> filenames = this->_print_sessions(curr_y);
        wrefresh(this->in);  

        curr_y+=2;
        wattron(this->in, COLOR_PAIR(1)), wattron(this->in, A_BOLD);
        mvwprintw(this->in, curr_y, 2, "[PRINT SESSION (q=QUIT/a=all)]:");
        wattroff(this->in, A_BOLD), wattroff(this->in, COLOR_PAIR(1));
        char ch[10];
        wgetstr(this->in, ch);                            
        string s(ch);

        try{
            if(s=="q"){
                wclear(this->in), wrefresh(this->in);
                return;
            }

            if(s==""){
                s="a";
            } else if(s!="a"){
                stoi(s);
            }
        }catch(...){
            this->makealert("WRONG INPUT (press any key)");
            getchar();            
            wclear(in), wrefresh(in);        
            return;
        }        

        int i=0;
        try{            
            //string fname="file1.txt"; // fname=filenames[stoi(s)];
            
            while(i<filenames.size()){                

                if(filenames[i]=="." || filenames[i]==".."){
                    i++;
                    continue;
                }
                
                string fname = "./docs/file1.txt";
                ifstream file(fname.c_str());

                if(!file){
                    i++;
                    continue;
                }

                string text;
                //file >> text;

                this->makealert("READ ONLY [ SESSION NO. : "+to_string(stoi(s))+ " ] (press any key)");            
                wclear(in), wrefresh(this->in), wmove(in, 1, 1);  

                int i=0, curr_y=1, curr_x=1;
                wattron(this->in, COLOR_PAIR(3));
                mvwprintw(this->in, 0, 0, "%s",("  TITLE : "+fname).c_str());
                wattroff(this->in, COLOR_PAIR(3));

                this->_printnum(curr_y);                
                this->_writer(curr_y, curr_x, text);
                        
                wrefresh(in), getch();
                i++;
            }
            //flag=false;   
        }
        catch(...){
            makealert(("ERROR OCCURED (press any key) "+filenames[i]).c_str()), getchar();
        }

/*
        bool flag=true;
        List* ptr=head;                
        while(ptr!=NULL){

            if(s=="a" \\ stoi(s)==ptr->session)
            {
                this->makealert("READ ONLY [ SESSION NO. : "+to_string(ptr->session)+ " ] (press any key)");
                string text=ptr->data;
                wclear(in), wrefresh(this->in), wmove(in, 1, 1);            
                
                int i=0, curr_y=1, curr_x=1;
                wattron(this->in, COLOR_PAIR(3));
                mvwprintw(this->in, 0, 0, "%s",("  TITLE : "+ptr->title).c_str());
                wattroff(this->in, COLOR_PAIR(3));

                this->_printnum(curr_y);                
                this->_writer(curr_y, curr_x, text);
                      
                wrefresh(in);
                getch();
                flag=false;                
            }

            if(s!="a" && stoi(s)==ptr->session){
                break;
            }

            ptr=ptr->link;            
        }

        if(flag){
            this->makealert("SORRY! COULD NOT FIND SESSION : "+s+" (press any key)");
            getchar();
        }
*/
        wclear(this->in);
        wrefresh(this->in);
    }
       
    void insert(bool e_mode, List* ptr){        
        this->options(1);                
        wclear(this->in);               
        //
        string titletext, text="", alertmsg1, alertmsg2;
        if(e_mode){
            text=ptr->data;
            titletext="TITLE : "+ptr->title;            
            alertmsg1="CHANGES SAVED (press any key)";
            alertmsg2="EDITING [ SESSION : "+to_string(ptr->session)+" ]";
        }
        else{
            titletext="TITLE : ";
            alertmsg1="TEXT SAVED (press any key)";
            alertmsg2="WRITE [ SESSION : "+to_string(this->session_no)+" ]";
        }

        wattron(this->in, COLOR_PAIR(3));
        mvwprintw(this->in, 0,0, "%s",titletext.c_str());
        wattroff(this->in, COLOR_PAIR(3));
                
        char title[15];
        if(!e_mode)
            wgetstr(this->in, title);

        noecho();
        int curr_y=1, curr_x=1, c, ulcounter=5, rdcounter=0;

        this->_printnum(curr_y);
        
        if(e_mode){
            wmove(this->in, curr_y, curr_x+1);
            this->_writer(curr_y, curr_x, ptr->data);            
            makealert("EDITING [ SESSION : "+to_string(ptr->session)+" ]");
        }                            
        wrefresh(this->in);

        bool saved=false, redo_f=false, undo_f=false;                
        Stack undo, redo;
        string redo_str="";
        do{                          
            keypad(this->in, true);

            if(curr_x==getmaxx(in)-2){            
                curr_x=1, curr_y++;
                this->_printnum(curr_y);
            }

            if(ulcounter==2){
                this->makealert(alertmsg2);
            } else if(ulcounter>=5){
                this->makealert(alertmsg2+"+");                
                undo.push(text);
                ulcounter=0;
            }           

            c=wgetch(this->in);
            switch (c)
            {
                case ('x' & 0x1f):                                   
                                    if(!saved)
                                        this->makealert("COULD NOT SAVE SESSION (press any key)"), getchar();

                                    if(!e_mode && saved)
                                        this->session_no++;
                                        
                                    wclear(this->in), wclear(this->locs);
                                    wrefresh(this->in), wrefresh(this->locs);
                                    return;

                case ('u' & 0x1f):  if(undo.getTop()==NULL){
                                        this->makealert("UNDO STACK EMPTY");                                        
                                    }
                                    else{
                                        this->makealert(alertmsg2+"-");
                                        ulcounter=0, curr_y=1, curr_x=1;                                        
                                        
                                        wclear(this->in);
                                        wattron(this->in, COLOR_PAIR(3));
                                        mvwprintw(this->in, 0,0, "%s",(titletext+( (!e_mode) ? title : "" )).c_str());
                                        wattroff(this->in, COLOR_PAIR(3));

                                        text=undo.pop();
                                        redo.push(text);
                                        this->_printnum(curr_y), this->_writer(curr_y, curr_x,text), wrefresh(this->in);
                                        undo_f=true;
                                    }
                                    break;

                case ('r' & 0x1f):  if(redo_str!="" && !undo_f){
                                        text.append(" "+redo_str);                                        
                                        waddch(this->in, ' '), curr_x++;
                                        int i=0;
                                        while(redo_str[i]!='\0'){
                                            if(redo_str[i]=='\n' || curr_x == getmaxx(in)-2){                                                
                                                curr_x=2, this->_printnum(++curr_y) , wmove(this->in, curr_y, curr_x);
                                            }
                                            else{
                                                waddch(this->in, redo_str[i]), curr_x++;
                                            }
                                            i++;
                                        }
                                        waddch(this->in, ' '), curr_x++;
                                        redo_f=true;
                                    }

                                    else if(undo_f && redo.getTop()!=NULL){
                                        curr_y=1, curr_x=1; 
                                        wclear(this->in);
                                        wattron(this->in, COLOR_PAIR(3));
                                        mvwprintw(this->in, 0,0, "%s",(titletext+( (!e_mode) ? title : "" )).c_str());
                                        wattroff(this->in, COLOR_PAIR(3));

                                        text=redo.pop();
                                        undo.push(text);
                                        this->_printnum(curr_y), this->_writer(curr_y, curr_x,text), wrefresh(this->in);                                       
                                    }
                                    break;

                case ('o' & 0x1f):  try{                                  
                                        ofstream file;
                                        string fname(title);
                                        fname = "./docs/"+fname+".txt";
                                        file.open(fname);                                                                        
                                        file<<text;
                                        file.close();
                                        makealert(alertmsg1), saved=true, getchar(), makealert(alertmsg2);
                                    }catch(...){
                                        makealert("INTERNAL ERROR: couldn't save (press any key)");
                                        getchar();
                                        makealert(alertmsg2);
                                    }
                                    break;
                case '\n':
                case KEY_ENTER   :  curr_y++, curr_x=1;                
                                    text.push_back('\n');                
                                    this->_printnum(curr_y);
                                    wmove(in,curr_y, curr_x);     
                                    break;

                case '\b':
                case KEY_BACKSPACE:
                                    if(curr_x==1){                
                                        if(curr_y!=1){
                                            curr_y--, curr_x=getmaxx(in)-1;
                                            wmove(this->in, curr_y, curr_x);  

                                            char ch = ' ';
                                            while(ch == ' ' && curr_x != 2){
                                                wmove(this->in, curr_y,--curr_x), ch=winch(this->in);
                                            }
                                        }
                                    }
                                    else{
                                        text.pop_back(), mvwdelch(this->in, curr_y, curr_x--);
                                    }
                                    break;
                
                case KEY_LEFT:
                case KEY_RIGHT: break;

                default:    wmove(this->in, curr_y, ++curr_x), text.push_back(c), waddch(this->in,c), redo_str.push_back(c);
                            ulcounter++, rdcounter++;

                            if(c==' '){
                                redo_str="";
                            }
                            else if(redo_f){
                                redo_f=false, redo_str.push_back(' ');
                            }

                            if(undo_f){
                                undo_f=false;
                                redo.clean();
                            }

                            /*
                            if(redo_str.length()>5){
                                redo_str.erase(0,1), redo_str.erase(redo_str.size() - 1), redo_str.push_back(c), rdcounter=0;
                            }*/                            
            };

            locwin(curr_y, curr_x);            
            wrefresh(this->in);
        }while(true);
    }

    void del(){
        do{        
            if(head==NULL){
                this->makealert("NOTHING TO DELETE (press any key)");
                getchar();
                break;
            }

            wclear(this->in);
            wrefresh(this->in);

            int curr_y=2;
            this->_print_sessions(curr_y);
            curr_y+=2;
            wattron(this->in, COLOR_PAIR(1)), wattron(this->in, A_BOLD);
            mvwprintw(this->in, curr_y, 2, "[DELETE SESSION (q=QUIT)]:");
            wattroff(this->in, COLOR_PAIR(1)), wattroff(this->in, A_BOLD);
            char ch[10];
            wgetstr(this->in, ch);
            
            if(ch[0]=='q')
                break;
            
            string s(ch);
            try{
                stoi(s);
            }catch(...){
                this->makealert("WRONG INPUT (press any key)");
                getchar();
                this->makealert("PLEASE CHOOSE AN OPTION");
                continue;                
            }

            curr_y++;
            wattron(this->in, COLOR_PAIR(1)), wattron(this->in, A_BOLD);
            mvwprintw(this->in, curr_y, 2, "[SURE ? (Y/n/q=QUIT)]:");
            wattroff(this->in, COLOR_PAIR(1)), wattroff(this->in, A_BOLD);        
            int confirm=wgetch(this->in);

            //
            if(confirm=='n'){
                wrefresh(this->in); 
                continue;
            }
            //
            if(confirm=='q'){
                break;
            }            
            //
            if(confirm=='y'){
                bool flag=false;

                if(head->session==stoi(s)){
                    List* temp = head;                    
                    head=head->link;
                    delete temp;   
                    flag=true;                 
                }

                else{
                    List* prev=head;
                    List* ptr=head->link;
                    while(ptr!=NULL){
                        if(ptr->session==stoi(s)){
                            List* temp = ptr;
                            prev->link = ptr->link;
                            delete temp;
                            flag=true;
                            break;
                        }
                        prev=ptr, ptr=ptr->link;                        
                    }                   
                }

                if(flag)
                    this->total_sessions--;

                string str = (flag) ? "SESSION : "+s+" DELETED (press any key)" : "SORRY! COULD NOT FIND SESSION : "+s+" (press any key)";
                this->makealert(str);
                
                getchar();                
                wclear(this->alert);
                wrefresh(this->alert);                
            }
 
            wrefresh(this->in);                
        }while(true);
    
        wclear(this->in);
        wrefresh(this->in);            
    }

    void search(){

        if(head==NULL){
            this->makealert("NOTHING TO SEARCH FROM (press any key)");
            getchar();
            return;
        }

        int curr_y=2;
        wclear(this->in);
        
        wattron(this->in, COLOR_PAIR(1)), wattron(this->in, A_BOLD);
        mvwprintw(this->in, curr_y, 2, "[STRING]:");
        wattroff(this->in, COLOR_PAIR(1)), wattroff(this->in, A_BOLD);
        wrefresh(this->in);

        char str[20];
        wgetstr(this->in, str);
        string s(str);
        
        if(s==""){
            this->makealert("EMPTY STRING ENTERED (press any key)");
            getchar();
            wclear(this->in);
            wrefresh(this->in);
            return;
        }

        this->makealert("SEARCHING IN ALL SESSIONS....");

        List* ptr=head;       
        bool flag=true; 
        curr_y+=2;
        while(ptr!=NULL){
            int n = ptr->data.find(s);
            int n2 = ptr->title.find(s);

            if(n!=string::npos || n2!=string::npos){
                wattron(this->in, COLOR_PAIR(3));
                if(n!=string::npos && n2!=string::npos){                                
                    string locs=" (body & title) [LINE:"+to_string(this->getcol(ptr->data,n))+"] [COL:"+to_string(n2)+"]";
                    mvwprintw(this->in, curr_y, 2, "%s", ("FOUND IN SESSION : "+to_string(ptr->session)+locs).c_str());
                }
                else if(n!=string::npos){
                    string loc=" (body) [LINE:"+to_string(this->getcol(ptr->data,n))+"]";
                    mvwprintw(this->in, curr_y, 2, "%s", ("FOUND IN SESSION : "+to_string(ptr->session)+loc).c_str());
                }
                else if(n2!=string::npos){
                    string loc=" (title) [COL:"+to_string(n2)+"]";
                    mvwprintw(this->in, curr_y, 2, "%s", ("FOUND IN SESSION : "+to_string(ptr->session)+loc).c_str());
                }
                wattroff(this->in, COLOR_PAIR(3));
                curr_y++, flag=false;
            }                        
            ptr=ptr->link;
        }
        
        if(flag){
            wattron(this->in, COLOR_PAIR(1));
            mvwprintw(this->in, curr_y, 2, "COULD NOT FIND IN ANY SESSION");            
            wattroff(this->in, COLOR_PAIR(1));
        }

        wrefresh(this->in);
        
        this->makealert("SEARCH COMPLETE (pess any key)");
        getchar();
        wclear(this->in);
        wrefresh(this->in);
    }

    int getcol(string s, int n){
        int i=0, col=1;
        while(i<s.length()){            
            if(i==n)
                break;

            if(s[i]=='\n')
                col++;
            i++;
        }

        return col;
    }

    void edit(){
        wclear(this->in);
        if(head==NULL){
            this->makealert("NOTHING TO EDIT (press any key)");
            getchar();
            return;
        }

        int curr_y=2;
        this->_print_sessions(curr_y);
        curr_y+=2;
        wattron(this->in, COLOR_PAIR(1)), wattron(this->in, A_BOLD);
        mvwprintw(this->in, curr_y, 2, "[EDIT SESSION (q=QUIT)]:");
        wattroff(this->in, COLOR_PAIR(1)), wattroff(this->in, A_BOLD);
        wrefresh(this->in);
        char ch[15];
        wgetstr(this->in, ch);

        string s(ch);

        if(s==""){
            this->makealert("EMPTY INPUT (press any key)");
            getchar();
            wclear(this->in), wrefresh(this->in);
            return;
        }
        //
        else if(ch[0]=='q'){
            wclear(this->in), wrefresh(this->in);
            return;
        }
        
        try{
            stoi(s);
        } catch(...){
            this->makealert("WRONG INPUT (press any key)");
            getchar();
            wclear(this->in), wrefresh(this->in);
            return;
        }

        bool flag=true;
        List* ptr=head;
        while(ptr!=NULL){
            if(ptr->session==stoi(s)){
                flag=false;
                break;
            }
            ptr=ptr->link;
        }

        if(flag){            
            this->makealert("SORRY! COULD NOT FIND SESSION : "+s+" (press any key)");
            getchar();
        } else{
            this->insert(true, ptr);
        }

        wclear(this->in), wrefresh(this->in);        
    }
    
    void curr_window(string wname){
        if(curr!=NULL){
            wclear(curr);
            wrefresh(curr);
        }
        //
        int x = getmaxx(stdscr);
        
        int strlen = wname.length();
        int start_x = x/2-strlen/2;
        
        curr = newwin(1,strlen,this->y-2,start_x);                
        wrefresh(curr);
        wattron(curr, COLOR_PAIR(6)), wattron(curr, A_BOLD);
        wprintw(curr, "%s", wname.c_str());
        wattroff(curr, A_BOLD), wattroff(curr, COLOR_PAIR(6));
        wrefresh(curr);        
    }
};

int main(){
    initscr();
    cbreak();
    
    Menu m;    
    int ch;    
    do{      
        echo();  
        m.options(0);
        m.curr_window("M E N U");
        m.makealert("PLEASE CHOOSE AN OPTION");        
        ch=m.choice();        

        switch(ch){
            case 'q':
            case '0':   m.curr_window("E X I T");
                        m.makealert("BYE (press any key)");
                        getchar();
                        break;

            case 'p':
            case '1':   m.curr_window("P R I N T");
                        m.print();
                        break;
            
            case 'w':
            case '2':   m.curr_window("I N S E R T");
                        m.makealert("WRITE [ SESSION : "+to_string(m.session_no)+" ]");
                        m.insert(false, NULL);                        
                        break;

            case 'd':
            case '3':   m.curr_window("D E L E T E");
                        m.del();
                        break;

            case 'e':
            case '4':   m.curr_window("E D I T");
                        m.edit();
                        break;

            case 's':
            case '5':   m.curr_window("S E A R C H");
                        m.search();
                        break;

            default :   m.makealert("PLEASE CHOOSE FROM GIVEN OPTIONS (press any key)");
                        getchar();
        };    
    }while(ch!='q' && ch!='0');

    refresh(), endwin();
    return 0;
}