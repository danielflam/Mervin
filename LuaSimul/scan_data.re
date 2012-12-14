#include "scan_data.h"

#define YYCTYPE unsigned char;
#define YYFILL(n)  charBuf->fill(instream) 

#define YYGETCONDITION() currentCondition
#define YYSETCONDITION(c) currentCondition = c

struct CharBuf
{
	CharBuf()
	{
		cursor = buffer;
		limit = &buffer[2047];

	}

	fill(istream & instream)
	{  
		int n = limit - cursor;

		if (n < 0)
			n=0;

		if (istream.eof())
		{
			cursor = buffer;
			*cursor = '\0';
		}
		else
		{
			memmove( buffer, cursor, n);
			cursor = buffer + n;			
			// by using get we guarantee that there is a null terminator at eof!
			istream.get(cursor, sizeof(buffer)-n);
		}
	}

	YYCTYPE buffer[2049];
	YYCTYPE * cursor;
	YYCTYPE * limit;
};


void scanData(istream & instream, OnDataReady * onDataReady)
{
	enum YYCONDTYPE {
		yycSYM,
		yycARG,
		yycARG_DECIMAL,

	} currentCondition;

	CharBuf charBuf;
	int cmdPtr;
	TickDataStruct tickData;

beginning:
	reset();

	/*!re2c
			re2c:define:YYCURSOR = charBuf->cursor;
			re2c:yyfill:enable = 1;
			re2c:indent:top = 1;
			re2c:define:YYLIMIT = charBuf->limit;
			re2c:yyfill:parameter = 0;
			EOF = [\X0000];

			CR = '\r';
			LF = '\n';

			eoc = ";";
            eof = "\000";
            digit = [0-9];
			number = digit+;
			plusminus = [+-];
			integer = plusminus? digit+;
            alpha = [A-Za-z_];
            space = " ";
			other = [^];
			period = ".";
			sym = alpha | "/" | "-";


			<SYM>     sym  { 
						if ( cmdPtr >= MAXCMDLEN ) 
						{
							signalError(tickData);
							return;
						}
						else
						{
							tickData.cmd[cmdPtr++] = yych;
							tickData.cmd[cmdPtr] = '\0';
							goto yyc_SYM;
						}
			}
			<SYM>   eof {return;}
			<SYM>   space* "," space* (digit|"-"){
				    if (cmdPtr > 0)
					{
						if (yych=='-')
						{
							isnegative = true;
							tickData.args.asArray[tickData.nArgs++] = 0;
						}
						else
						{
							isnegative = false; 
							tickData.args.asArray[tickData.nArgs++] = yych - '0';
						}
						YYSETCONDITION(yycARG); 
						goto yyc_ARG;
					}
					goto yyc_SYM;
			}
			<SYM> other {
					signalError(tickData);
					return YYCURSOR;
			}


 			<ARG> digit {
				tickData.args.asArray[tickData.nArgs-1] *= 10;
				if (isnegative)
				{
					tickData.args.asArray[tickData.nArgs-1] -= yych - '0';
				}
				else
				{
					tickData.args.asArray[tickData.nArgs-1] += yych - '0';
				}
				goto yyc_ARG;
			}
			<ARG> space digit { 
				isnegative = false;
				tickData.args.asArray[tickData.nArgs++] = yych - '0';
				goto yyc_ARG;
			}
			<ARG> space "-" {
				tickData.args.asArray[tickData.nArgs++] = 0;
				isnegative = true;
				goto yyc_ARG;
			}
			<ARG> space{ 
				goto yyc_ARG;
			}

			<ARG> eof { 
				return;
			}
			<ARG> other {
					signalError(tickData);
					return;
			}
			
 			<ARG_DECIMAL> digit {
				tickData.args.asArray[tickData.nArgs-1] *= 10;
				if (isnegative)
				{
					tickData.args.asArray[tickData.nArgs-1] -= yych - '0';
				}
				else
				{
					tickData.args.asArray[tickData.nArgs-1] += yych - '0';
				}
				goto yyc_ARG;
			}
			<ARG_DECIMAL> space digit { 
				isnegative = false;
				tickData.args.asArray[tickData.nArgs++] = yych - '0';
				goto yyc_ARG;
			}
			<ARG_DECIMAL> space "-" {
				tickData.args.asArray[tickData.nArgs++] = 0;
				isnegative = true;
				goto yyc_ARG;
			}
			<ARG_DECIMAL> space{ 
				goto yyc_ARG;
			}

			<ARG_DECIMAL> eof { 
				return;
			}
			<ARG_DECIMAL> other {
					signalError(tickData);
					return;
			}

*/
}