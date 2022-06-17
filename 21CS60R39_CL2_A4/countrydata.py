import ply.yacc as yacc
import ply.lex as lex
from datetime import datetime
class Country:
    def __init__(self):
        self.dict1={}
        self.tokens = [
        'ACTIVE',
        'CAT',
        'DEATH',
        'NEW',
        'RECOVER',
        'DATA',
    ]

    def t_SCRIPT(self, t):
        r'<\/?script[a-zA-Z=\'"\/ ]*>'
        pass

    def t_ACTIVE(self, t):
        r"title:.\{[\t\n ]*text:\s'Active\sCases'"
        return t

    def t_DEATH(self, t):
        r"title:.\{[\t\n ]*text:\s'Daily\sDeaths'"
        return t
    
    def t_NEW(self,t):
        r"title:.\{[\t\n ]*text:\s'Daily\sNew\sCases'"
        return t

    def t_RECOVER(self, t):
        r"series:.\[\{[\t\n ]*name:\s'New\sRecoveries'"
        return t

    def t_CAT(self, t):
        r'categories:\s\[[a-zA-Z0-9,\" ]+\]'
        return t

    def t_DATA(self, t):
        r'data:.\[[0-9a-z\-\.,]*\]'
        return t
    

    def p_start(self, t):
        '''start : pactive
                | pdeath
                | precover 
                | pnew
                '''

    def str_date(self, msg):
        dates = msg.partition('[')[2][:-1].replace('"', '').split(',')
        arr = []
        j = 0
        for i in range(0, len(dates), 2):
            arr.append(dates[i]+dates[i+1])
        dates = [datetime.strptime(i, '%b %d %Y').date() for i in arr]
        return dates

    def str_value(self, msg):
        data = msg.replace('null', '0')
        data = data.partition('[')[2][:-1].split(',')
        data = [float(i) for i in data]
        return data

    def p_pactive(self, t):
        'pactive : ACTIVE CAT DATA'
        date = self.str_date(t[2])
        data = self.str_value(t[3])
        self.dict1['Active Cases'] = dictionary = dict(zip(date, data))

    def p_pdeath(self, t):
        'pdeath : DEATH CAT DATA'
        date = self.str_date(t[2])
        data = self.str_value(t[3])
        self.dict1['Daily Deaths'] = dict(zip(date, data))

    def p_precover(self, t):
        'precover : CAT RECOVER DATA'
        date = self.str_date(t[1])
        data1 = self.str_value(t[3])
        self.dict1['New Recovered'] = dict(zip(date, data1))
    
    def p_pnew(self,t):
        'pnew : NEW CAT DATA'
        date = self.str_date(t[2])
        data = self.str_value(t[3])
        self.dict1['New Cases'] = dict(zip(date, data))

    def p_error(self, t):
        pass

    def t_error(self, t):
        t.lexer.skip(1)

    def crawl(self,file_name):
        text=open(file_name,'r',errors='ignore').read()
        self.lexer = lex.lex(object=self)
        self.lexer.input(str(text))
        #for tok in self.lexer:
        #    print(tok.type, tok.value)
        self.parser = yacc.yacc(module=self)
        self.parser.parse(text)
        return self.dict1


#covid=Country()
#covid.crawl('UK.html')
#print(covid.dict1.keys())

