#include <iostream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

string phrase = "srkc kc rgl sg ifyq f jggh iqhkakwq mgd f agph srkc kc rgl sg ifyq f jggh iqhkakwq sg srdgl flfx f arkph nqmgdq ks qvqw nqagiqc f arkph srkc kc rgl sg afsar f mkcr srkc kc rgl sg srdgl nfay f mkcr xgz hgws pkyq fwh srfs lfx cgiqsrkwj nfh lgws mfpp gw xgz srkc kc rgl sg nzppx f ifw srkc kc rgl f ifw nzppkqc xgz srkc kc rgl sg pgvq f ifw fwh km srkc hgqcws lgdy srqdq fdq gsrqd lfxc fwh km srqx hgws lgdy hgws mqqp sgg nfh fngzs jkvkwj ze srkc kc rgl sg ceks ze kw srq fkd km xgz mqqp pkyq ks fwh srkc kc rgl sg igvq uzkay cg srfs ks hgqcws mfpp gw xgz srkc kc rgl sg ifyq qwhc iqqs fplfxc cuzqqbq ndqfh sg ifyq czdq ksc mdqcr nzs lrfs km srq nfyqd lgws pqs iq mqqp srq ndqfh xgz iqfw sg cfx srfs fmsqd fpp xgz fdq dqfppx jgkwj sg nq srq ykwh gm lgifw lrg srq nfyqd lgws pqs wqfd srq ndqfh";

string replace(string word, string replacement){
    stringstream ss(phrase);
    string newLine = "", line;
    while(ss >> line){
        if(line == word){
            newLine += replacement;
        }else{
            newLine += line;
        }
        newLine += " ";
    }
    return newLine;
}

void replaceLetters(map<char,char> myMap, string &input){
    for(int i = 0; i < input.size();++i){
        if(myMap.find(input[i]) != myMap.end()){ // letter was found in key
            input[i] = myMap[input[i]];
        }
    }
}

int main(){
    // Make a map with the letter I already know
    map<char,char> m;
    // replace all the words that I know
    // phrase = replace("srkc","this");
    m.insert(pair<char,char>('s','t')); m.insert(pair<char,char>('r','h')); m.insert(pair<char,char>('k','i')); m.insert(pair<char,char>('c','s'));
    // phrase = replace("kc","is");
    // phrase = replace("rgl","how");
    m.insert(pair<char,char>('g','o')); m.insert(pair<char,char>('l','w'));
    // phrase = replace("sg","to");
    // phrase = replace("f","a");
    m.insert(pair<char,char>('f','a'));
    // phrase = replace("srq","the");
    m.insert(pair<char,char>('q','e'));
    // phrase = replace("cg","so");
    // phrase = replace("ks","it");
    // phrase = replace("lrg","who");
    // phrase = replace("fdq","are");
    m.insert(pair<char,char>('d','r'));
    // phrase = replace("lrfs","what");
    // phrase = replace("srfs","that");
    // phrase = replace("lgws","wont");
    // phrase = replace("fwh","and");
    m.insert(pair<char,char>('w','n'));
    // phrase = replace("lfx","way");
    m.insert(pair<char,char>('x','y'));
    m.insert(pair<char,char>('m','f'));
    m.insert(pair<char,char>('n','b'));
    m.insert(pair<char,char>('a','c'));
    m.insert(pair<char,char>('i','m'));
    m.insert(pair<char,char>('h','d'));
    m.insert(pair<char,char>('j','g'));
    m.insert(pair<char,char>('y','k'));
    m.insert(pair<char,char>('z','u'));
    m.insert(pair<char,char>('p','l'));
    m.insert(pair<char,char>('u','q'));
    m.insert(pair<char,char>('e','p'));
    m.insert(pair<char,char>('b','z'));
    // phrase = replace("flfx","away");
    // phrase = replace("gw","on");
    // phrase = replace("xgz","you");
    // phrase = replace("cfx","say");
    // phrase = replace("lfxc","ways");
    // phrase = replace("lgdy","word");
    // phrase = replace("ksc","its");
    // phrase = replace("afsar");
    replaceLetters(m,phrase);
    cout << phrase << endl;
    return 0;
}