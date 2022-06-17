
import ply.yacc as yacc
import ply.lex as lex

class World:
    def __init__(self):
        self.dict1={}
        self.tokens = [
        'LYES',
        'LCON',
        'RCON',
        'LTD',
        'RTD',
        'LWORLD',
        'RWORLD',
        'LCOU',
        'RCOU',
        'NA',
        'NUM',
        'NAME',
    ]


    def t_LYES(self,t):
        r'<div.class="tab-pane\s".id="nav-yesterday".role="tabpanel".aria-labelledby="nav-yesterday-tab">'
        return t


    def t_LCON(self,t):
        r'<tr.class="total_row_world\srow_continent".data-continent="[a-zA-Z\/\-_ ]+".style="display:\snone">'
        return t


    def t_RCON(self,t):
        r'<tr.class="total_row_world\srow_continent".data-continent="".style="display:\snone">'
        return t


    def t_LWORLD(self,t):
        r'<tr.class="total_row_world">'
        return t


    def t_RWORLD(self,t):
        r'<td.style="display:none".data-continent="all">'
        return t


    def t_LTD(self,t):
        r'<td[A-Za-z0-9\/:\-()="_!\t\n,;%&# ]*>'
        return t


    def t_RTD(self,t):
        r'<\/td[A-Za-z0-9\/:\-()="_!\t\n,;%&# ]*>'
        return t


    def t_LCOU(self,t):
        r'<tr.style=[A-Za-z0-9\/:\-()="_!\t\n,;%&# ]*>'
        return t


    def t_RCOU(self,t):
        r'<\/tbody>'
        return t


    def t_NA(self,t):
        r'N\/A'
        return t


    def t_NUM(self,t):
        r"[+\-0-9][0-9,]*[.]?[0-9]*"
        return t


    def t_NAME(self,t):
        r"[A-Za-z':,.\/\-(_)#=()]+"
        return t


    def t_ANY(self,t):
        r'<[A-Za-z0-9\/:\-()="_!\t\n,;%&# ]+>'
        pass


    t_ignore = " \t"


    def t_error(self,t):
        t.lexer.skip(1)


    def p_start(self,t):
        '''start : LYES extra continent_data RCON extra world_data RWORLD extra country_data RCOU'''



    def p_world_data(self,t):
        'world_data : LWORLD val val val val val val val val val val val val val val val'
        self.dict1[t[3]] = {}
        self.dict1[t[3]]['Total Cases'] = t[4]
        self.dict1[t[3]]['New Cases'] = t[5]
        self.dict1[t[3]]['Total Deaths'] = t[6]
        self.dict1[t[3]]['New Deaths'] = t[7]
        self.dict1[t[3]]['Total Recovered'] = t[8]
        self.dict1[t[3]]['New Recovered'] = t[9]
        self.dict1[t[3]]['Active Cases'] = t[10]
        self.dict1[t[3]]['Deaths/M']=t[13]


    def p_country_data_multi(self,t):
        'country_data : LCOU val val val val val val val val val val val val val val val val val val val val val val country_data'
        self.dict1[t[3]]={}
        self.dict1[t[3]]['Total Cases'] = t[4]
        self.dict1[t[3]]['New Cases'] = t[5]
        self.dict1[t[3]]['Total Deaths'] = t[6]
        self.dict1[t[3]]['New Deaths'] = t[7]
        self.dict1[t[3]]['Total Recovered'] = t[8]
        self.dict1[t[3]]['New Recovered'] = t[9]
        self.dict1[t[3]]['Active Cases'] = t[10]
        self.dict1[t[3]]['Deaths/M']=t[13]
        self.dict1[t[3]]['Total Tests']=t[14]
        self.dict1[t[3]]['Tests/M']=t[15]


    def p_country_data(self,t):
        'country_data : LCOU val val val val val val val val val val val val val val val val val val val val val val'
        self.dict1[t[3]]={}
        self.dict1[t[3]]['Total Cases'] = t[4]
        self.dict1[t[3]]['New Cases'] = t[5]
        self.dict1[t[3]]['Total Deaths'] = t[6]
        self.dict1[t[3]]['New Deaths'] = t[7]
        self.dict1[t[3]]['Total Recovered'] = t[8]
        self.dict1[t[3]]['New Recovered'] = t[9]
        self.dict1[t[3]]['Active Cases'] = t[10]
        self.dict1[t[3]]['Deaths/M']=t[13]
        self.dict1[t[3]]['Total Tests']=t[14]
        self.dict1[t[3]]['Tests/M']=t[15]

    def p_continent_data_multi(self,t):
        'continent_data : LCON val val val val val val val val val val val val val val val val val val val val val val continent_data'
        self.dict1[t[17]] = {}
        self.dict1[t[17]]['Total Cases'] = t[4]
        self.dict1[t[17]]['New Cases'] = t[5]
        self.dict1[t[17]]['Total Deaths'] = t[6]
        self.dict1[t[17]]['New Deaths'] = t[7]
        self.dict1[t[17]]['Total Recovered'] = t[8]
        self.dict1[t[17]]['New Recovered'] = t[9]
        self.dict1[t[17]]['Active Cases'] = t[10]




    def p_continent_data(self,t):
        'continent_data : LCON val val val val val val val val val val val val val val val val val val val val val val'
        self.dict1[t[17]] = {}
        self.dict1[t[17]]['Total Cases'] = t[4]
        self.dict1[t[17]]['New Cases'] = t[5]
        self.dict1[t[17]]['Total Deaths'] = t[6]
        self.dict1[t[17]]['New Deaths'] = t[7]
        self.dict1[t[17]]['Total Recovered'] = t[8]
        self.dict1[t[17]]['New Recovered'] = t[9]
        self.dict1[t[17]]['Active Cases'] = t[10]


    def p_extra(self,t):
        '''extra : NAME
                | NUM
                | NA
                | LTD
                | RTD'''


    def p_extra_multi(self,t):
        '''extra : NAME extra
                | NUM extra
                | NA extra
                | LTD extra
                | RTD extra'''


    def p_val1(self,t):
        'val : LTD mid RTD'
        t[0] = t[2]
    

    def p_val2(self,t):
        'val : LTD RTD'
        t[0] = "NA"


    def p_mid(self,t):
        '''mid : pnum 
                | pname
                '''
        t[0] = t[1]


    def p_pnum(self,t):
        '''pnum : NUM
                | NA
                '''
        if t[1] == 'N/A':
            t[0] = "NA"
        else:
            t[1] = t[1].replace('+', '')
            t[1] = t[1].replace(',', '')
            t[0] = float(t[1])


    def p_pname_multi(self,t):
        'pname : NAME pname'
        t[0] = t[1]+' '+t[2]


    def p_pname(self,t):
        'pname : NAME'
        t[0] = t[1]


    def p_error(self,t):
        pass
    
    def crawl(self,file_name):
        text=open(file_name,'r',errors='ignore').read()
        self.lexer = lex.lex(object=self)
        self.lexer.input(str(text))
        #for tok in self.lexer:
        #    print(tok.type, tok.value)
        self.parser = yacc.yacc(module=self)
        self.parser.parse(text)
        return self.dict1
        

#covid=World()
#covid.crawl('main.html')
#print(len(covid.dict1.keys()))
#print(covid.dict1)