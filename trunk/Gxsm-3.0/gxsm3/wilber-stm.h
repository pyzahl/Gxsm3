/*  GIMP header image file format (RGB): /home/percy/C/Gxsm/src/wilber-stm.h  */

static unsigned int wilberstm_width = 77;
static unsigned int wilberstm_height = 61;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define HEADER_PIXEL(data,pixel) {\
  pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
  pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
  pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
  data += 4; \
}
static const char *wilberstm_data =
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6T.03L]L*>,CY-[3E%JO<(*[>/+;G"
	".[;F(:[?(Z_?3KSLC<__O=X.T^44V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6S>,2K=H);L7V-+3D':W>(:[?-+3D,+/C'ZW>(:[?"
	"0KCI@<O\\MMP,T.03V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6S.,2JMD(:\\7U,++C&*O<'JW>,+/C+;+B':W>'JW>/K?H?<KZM=P+"
	"T>44V><6V><6V><6V><6V><6R=+^V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"SN03J]D(:L3U,;/D&ZS='ZW>,+/C++'B'*S='JW>/;;G?,KZM=P+T>44V><6V><6"
	"V><6V><6V><6J+'5R<[^V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6S>,2J]D(:L3U"
	",+/C&JS='JW>,++C++'B':W>'JW>/+;G>LGZM-P+T.03V><6V><6V><6V><6T=L&"
	"?X6CN,+MV>,6V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V=[^V=\\&V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6S>,2JMD(:\\7U,+/C&JS='ZW>"
	",++C++'B&JS='*S=/+;G><GYL]P+T.03V><6V><6V><6V>,6J*W5?X6CN,+MV>,6"
	"V>,6V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6T<[=V>,.V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6SN03J]D(:L3U,;/D&JS='ZW>,++C*['B'*S="
	"'JW>.[;F><GYM-P+T>44V><6V>,6V>,6P<[^?X6C?X&;N+[MT=\\.V>,6V>,6V>,6"
	"V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6T=+MR<KEV>,.V><6V>,6V>,6V>,6V>,6"
	"V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6N+[M5E1Z"
	"V><6V><6V><6V><6S.,2JMD(:L3U+K+C&*O<':W>+[+C*;#A%ZO<&*O<-[7F>,CY"
	"M-P+T>44V>,6V>,6V>,6CY6\\?X&;=WV;H*G,T=\\.T=\\.V>,6V>,6V>,6V>,6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6T=K^R<;5P<+=V=\\.V>,6V>,6V>,6V>,6V>,6V>,6V>,6"
	"V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><614!A+2Q)V><6V><6V><6"
	"V><6S>,2JMD(:<3T,+/C&JS='JW>+[+C*['B'*S='JW>.[;F><GYL]P+T>$4V>,6"
	"V>,6H*G5?X&;=WV;=WV;F)F\\T=\\.T=\\.T=\\.V>,6V>,6V>,6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6P<+5N+K,N+K5T=\\.V>,6V>,6T=\\.T=\\.T=\\.T=\\.T=\\.T=\\.T=\\."
	"T=\\.V>,6T=\\.V>,6T=\\.T=\\.T=L.T=L.=WFC+2A)T=\\.T=\\.T=\\.T=\\.S=`2J=4("
	"9\\+T*[#B%*K;&JS=+[+C++'B&JS=&ZS=.K7F=LCXLM@*T.$3V>,6N+[E=WV;=WV;"
	"=WV;=WF;L+KET=L.T=L.T=\\.T=\\.V>,6V>,6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"R<KMN+K,L+'$J*G$T=<&T=\\.T=L.T=L.R=<&R=<&R=<&R=<&R=,&R=,&R=,&R=,&"
	"P<[^P<K^N,;UN,;UN,+UF*',+2A)N,;UP<K^R=,&P<[^OM$#GLO\\8KWM+K'B&JO<"
	"'ZW>,;+D,++C'JW>'JW>.+7F=,7XK]<*R-T,R=,&=WV;=WF;=WF3;GF3;G63J+7="
	"R=<&T=L.T=\\.T=\\.V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6T=L&L+7,L*W$"
	"H*&\\H*&\\R=+^R=<&R=,&P<[^P<K^F)F\\=W639F6\"/31)/31)/31)-3!)-3!)-3!)"
	"-2Q)+2Q)+2A)+2A)3E!Z7F\"+=X&K?X6T>(>W?:S<6;3F*Z[?%ZK;':S=,K'C,K+C"
	"'ZW>&JS=-+/D<,3VJ-0#P-$$?XFC=WF;=WF3;G63;G63;G63J+'=R=,&R=<&T=L."
	"T=\\.V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6P<+=L*W$H*6\\F)FTCY6T"
	"J+7=H*G5;FF\"13Q113A113A)/31)/31)/31)/3!)-3!)-3!)-2Q)+2Q)+2Q)+2A)"
	"+2A))\"A))\"1))\"1)'\"!!&RE+&4)G%FZ:%IK*%:?7'ZO<-+#A-++B'ZW>&:O<,++B"
	":,'RF;_O>8>F=WF3;G63;G63;G&3;G&39G&+L+KER<[^R=,&T=L.T=\\.V>,6V>,6"
	"V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V>,6V>,6V>,6R<[UL*W$H*6\\CY6T?X&;5DQA3CQ113Q1"
	"13A113A)/3A)/31)/31)/31)-3!)-3!)-2Q)-2Q)+2Q)+2A)+2A))\"A))\"1))\"1)"
	")\"1!'\"!!&RI,&4-I$G&=$9C($:75%J'1-:[@-+#A'ZW=&ZS=,+'B6;'?8(>I<7R7"
	";GF3;G63;G&39FV+9FV+9FV+P<K^P<[^R=,&R=<&T=L.T=\\.V>,6V>,6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V>,6V>,6T=\\.T=\\.L+',H*6\\?WF33D!13CQ113Q113Q113A113A)/31)"
	"/31)/31)-3!)-3!)-3!)-2Q)+2Q)+2Q)+2A)+2A))\"A))\"1))\"1)'\"!!'\"!!&RM-"
	"&4)J$G.@$9K)$:35$9S,&Y3%,Z[@'*O<%ZK<):?6/Y6^7X2D:'B7;G&39G&+9FV+"
	"9FF+9FF+=WVCN,+UP<K^P<[^R=<&T=L.T=\\.V>,6V>,6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6"
	"V>,6T=\\.T=L.N,+EH*6\\5DA93CQ13CQ113Q113A113A)/3A)/31)/31)/31)-3!)"
	"-3!)-2Q)+2Q)+2Q)+2A)+2A))\"A))\"1))\"1))\"1!'\"!!'\"!!&RI,$T-K$G2@$9G)"
	"$:35$9O,$(>W,:S>'*K:%Z?7(:'0/I2[6(&D:7B7;G&39FV+9FF+7FF+7F6\"AXVT"
	"L+[MN,;UP<K^R=,&T=L.T=\\.V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=L."
	"R=+^J*F\\3D!13CQ113Q113A113A113A)/31)/31)/31)-3!)-3!)-3!)-2Q)+2Q)"
	"+2Q)+2A))\"A))\"1))\"1))\"1!'\"!!'\"!!'\"!!&RA.$T-L$G:B$9O+$:?8$9W-$HN["
	"))_-%J;6$JC9'J/1.9.[6'ZD:7279FV+9FF+7FF\"7F6\"7F6\"F*75L+KMN,+UP<K^"
	"R-(%R=8&T-L-T=\\.V>,6V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=L.R=,&L+K=5DA9"
	"3CQ113Q113A113A)/3A)/31)/31)/31)-3!)-3!)-2Q)+2Q)+2Q)+2A)+2A))\"A)"
	")\"1))\"1)'\"1!'\"!!'\"!!'\"!!%\"=-$T-K$G2B$9K*$:35%)_/(9?&(9S*&*34%:;7"
	"'Z'/.I\"[6'Z=87&79FV+7FF+7F6\"7F6\"9G&3J+'EJ+7EK[WLM\\7TP,W]R-8%T-H-"
	"T-X-V.,5V>,6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=<&R=,&P<K^?X&;13Q113A113A)"
	"13A)/31)/31)/31)-3!)-3!)-3!)-2Q)+2Q)+2Q)+2A))\"A))\"1))\"1))\"1!'\"!!"
	"'\"!!'\"!!'!Q!(CID'U!^'H*O%*#0$:G:%:/3()C'()O(%Z75%*?8'Z'/-Y\"Z4GN>"
	"86V09FF+7F6\"7F6\"7F\"\"?X6TH*G=I[#DKKSKML3SO\\S\\Q]4$S]D,T-X-V.(5V>,6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V>,6V>,6V>,6T=\\.R=<&R=,&P<K^L+7=AX6;7E1J13A)/3A)/31)/31)"
	"/3!)-3!)-3!)-2Q)+2Q)-311+3!1+3!1+319+3AA+31A+3AA-4!J-4!J+3AA+3AA"
	",D5J*UV#(82Q%9_/$Z;6%Z'1(9?$(9K'%Z33%:;6'J#.-XZW3'B=86V/7F6\"7F6\""
	"7F\"\"7FF+CYW,EZ34IJ_;K;?JM;_QO<KZQ<`\"SM@+S]T,U^(4V.(5V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V>,6V>,6T=\\.T=L.R=,&P<KUJ*W,F)VTCY&KAXFC=WF;9G&+7F6+5F\"\"5EQZ3EAZ"
	"3E1Z3E!Z15!R14QR14AR/4AJ/41J-41J-4!J-4!J-4!J-4!J-4!J.4QS,6.))(>T"
	"%J#0$JC8%J+2()C%(9K(%Z33%*;7'9_.-(JR3'6666F'7F\"\"7F\"\"5F\"\"?X6TCYG,"
	"EZ#4GJO;I+'ALKSONL3WPLS_R]8(S]P,UN$3V.(5V>86V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\."
	"T=L&XN\\?^O\\O``L`\\OLGR<[UCY&K?X6C=WV;;G639FV3?XFTH*75L+KEL+KMH*W="
	"CYG$;GFC3EAZ3E1Z3E1Z15!R14QR14QR14QR14QR05=Z-VB/)8>S&*#/$Z;7%Z'1"
	"(9?#()G'%J/3%*;6'9[,,(BR3'&7662'5F\"\"5F\"\"9FV3?XV\\AI3#EI_3G*79HZ[?"
	"K[KLN,'UP<O^RM4'S=L*UN$3V.(5V.85V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.\\OLO``````L`"
	"``L`^P<W^P<WV><.CXVKAXFC?X6CN,+MV>,6T=\\.T=\\.T=\\.T=L.R=,&R=,&H*G5"
	"7FF+7F6\"5F\"\"5EQZ5EQZ5EAZ3EAZ25^\"/6N4*(NV%Z#0$Z?7%Z+1'Y;#()C%%J/2"
	"%:75')W+,(>O1FZ04F\"'5EQZ5F\"\"=H\"J?HB[A8_\"BYG(F:/6H*W=K;?JML/SP<K^"
	"RM0'SML+U=\\2U^(4V.85V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V><6N,;UN,+UR=,&T=L.V><6V>,6V>,6T=\\.XN\\?````^P<W``L`^P<W^P<W^P<W"
	"^P<WR<[UCY&KT=L&\\OLO\\OLOZO,GXNL?XNL?XN\\?XN\\?XN\\?V><6N,+M;G63;G&3"
	"9FV+9FF+7F6\"7F\"\"4&:)0W&4*XNV%Z#0$Z;7%Z'1'Y;$'I?&%:/4$Z;7&IW,+(6O"
	"1FZ14F%`5F\"\"96R:;7BJ=8.R@XV`DIO&F*35I*[AK+?IML+SP,S]R=0&S-D)U=\\2"
	"UN(3V.85V><6V><6V><6V><6V><6V><6V><6V><6V><6T=\\.H*G=7FF;152\"152\""
	"3ER+;GVKP<K^V>,6V>,6P<KU^P,O^P<W``L`^P<WV><6R=,&XN\\?``L`\\OLGP<;E"
	"``L`^P<W^P,WZO<G\\OLO\\OLO\\O\\O\\OLO\\OLOZO<GZO,GJ*W,=WF3;G63;G&39FV+"
	"7F6\"6&6)0W2:*XNV&*#/$Z;7%Z'0'97\"')?%%:+2%*35&YS*+(2P0&N12EU`5622"
	"7&N9:W:H=(*Q@HV^D)K,EJ+3HJS?K+GIM\\/TP,S]R-0%SML+U-\\1UN03V.85V><6"
	"V><6V><6V><6V><6V><6V><6V><6T=\\.CYW,3EB+15\"\"152\"/4AZ+3AJ'\"Q9152\""
	"F*75N,+MJ*F\\````^P,W``L`P<[^N,+U+3QJ-4!RR=<&````ZN\\?````````^P,W"
	"^P,W^P<WZO<GV>,6V>,6ZO,G\\OLOXN\\?P<[^=WV;;GF3;G&39FV+9FF+6&N)0W2;"
	"*HNW%Z'1$JC8%:+1'93#&Y;%%*34$JC8&)W-*X2M/VB125R/4UZ066B69W.D<'ZL"
	"?HNZC9;*FJ/7HZ_@K+CINL7WP<W^R=4&T=T.U-`1U^44V.85V><6V><6V><6V><6"
	"V><6V><6V><6V><6F*7515\"\"9G&CAY7$AY7$;GFK3EB++3QJ)#!A/4QZ=WV;H)V\\"
	"^O\\O^P,W````5F63J+'E+3QJ!!!!+3QJ\\OLO^P,W``L```L`\\O\\O``L`V><6H*G="
	"9G6C+3AJ9G6CV>,6XNL?T=\\.=WV;;G63;G&39FF+7FF+6&:)0W&4)XJW%Z#0$Z;6"
	"%J'1'93\"&I;#%*'2$Z75&9O**(*L,V*105>%45R.6&:49'.A<7RM?XN\\BY;&EZ34"
	"I;#BL+SMN\\7XQ,`!RM8'T=P.U=`2U^04V.85V><6V><6V><6V><6V><6V><6V><6"
	"T=L.9G&C3ER+F*'5R=<&R=,&CYW,7FF;/4AZ)#1A)#1A3EB\"?X&CT=;^^P<W````"
	"%\"11)#1A!!1!!!!!#!Q)T=;^^O\\O^P<W^P<W\\O\\O``L`F*75N,;UN,;U-4!R#!Q)"
	"?XV\\V><6R=<&=WF3;G63;G&39FV+7F6\"4&:*0G\"6)XNX%:+1$:G:%:+2&Y3#&I7#"
	"%*/3$J;7%YO+(7RJ,V&1/U>%3UF,6V:79G&B<'RJ?(FXC)C)FZ78I[/DLKWOO,?Y"
	"Q<`\"RM<'TMT/U.$1V.85V.85V><6V><6V><6V><6V><6V><6V><6R=,&/4AZ7FF;"
	"J+'EV><6T=\\.F*757FV;/4AZ+3QJ+3QJ/4AZ5F\"+F)FTXN\\?^P<W9G&C#!Q)!!!!"
	"#!Q)=X6TCY6TV>,.^P,W^P,W^P,W^P<W9G&C?XF\\?XF\\'\"A9!!!!3ER+T=L.L+7E"
	";GF3;G&39FV+7FF+76\"\"3V6(0F^5)XJT%J#/$Z76%:#0')&^&Y.`%:'0$Z75%IK+"
	"'GVL,6\"115>(3%N)5V6386V<;GFJ?XJ[CIG+G*C9J[3HML#SOLK[QM,#S-D)TMX/"
	"U>,2V.85V.85V><6V><6V><6V><6V><6V><6V><6F*75+3QJ3ER+AY7$L+[MJ+7E"
	"?XF\\3ER+-41R-4!R-41R/4QZ3E2\"?X&;CY&KR=+^ZO,?=X&T15\"\"AY7$CY6KCY&K"
	"N+[E\\O\\O\\O\\O\\O\\O^P,W7FV;'\"A9%\"11!!!!!!!!7FV;R-8%EYS#;G63;7&296F+"
	"762!55N!3F&#.VV6)HJX%J#0$JC8%:#0')\"]')&^%:#/$J;7%)S-'X&P,&*105F'"
	"35J(5V.386R=;WJJ@HV^D9O-H*S=KKGKN,7UP<S^Q],$SMH+T]\\0UN,3V.85V><6"
	"V><6V><6V><6V><6V><6V><6V><6F*'5'\"Q9/4QZ;GFKAY'$?XF\\7FV;15\"\"-4!R"
	"-41R/4QZ152\"3ER+7FF+AXFKAXVKAXVKAXVKCY&KCY&KCY&KCY&KCXVKR=;^\\OLO"
	"ZO<GZO,GH*W=)#1A!!!!!!!!)#!AL+KMGZC4;G63;7\"296R*8V>)66!_4%M]1UZ\""
	".F^8(XJW%J#/$Z76%I[-'HVY'8Z\\%9_/$J;6%)W.((.S-&660UF)35F(5V.38V^?"
	"<W^P@X^`EJ+3I;#BLKWON\\?XPL[_R=8&SML+T]`0UN03V.45V><6V><6V><6V><6"
	"V><6V><6V><6V><6CYW,)#!A-4!R5F\"39G&C7FF;3EB+/4AZ/4AZ/4QZ152\"3ER+"
	"5F637F63=WV;?X6C?X6CAXFCAXFCAXVKAXVKAXVKAXVKAY&KP<KUXN\\?XNL?V><6"
	"H*W=3ER+5F\"3L+KMJ+#<=WF;;7\"296R*8V:)5U]^4E9Y355V1F\"%.'&7(8VY%J'1"
	"$Z75%YS,'HNX'(V[%)_/$J;6%9[/(86T,F:40UF)3EJ)6V>7:'.D>(.TBI;'G*?9"
	"JK3GM<'ROLK[Q-#`R]<(T-T-U.(1UN03V.85V><6V><6V><6V><6V><6V><6V><6"
	"V><6H*W=)#1A)#1A/4AZ152\"152\"/4QZ/4AZ15\"\"3EB+5F\"37FF;7FV;9G&;;G&3"
	"=WV;?WV;?X6C?XFCAXFCAXVCAXVCAXFCAXFC?X6CJ+'5N,+MT=L.V><6T=\\.N,+M"
	"F*'$;G63;7&396^*86B'6%]]3E=Z35-Y2E9[15^&-G&9(8Z[%:'1$Z75%YO*'HJX"
	"&9\"_$Z/3$:C9%*'2'XBX,V>70UN+4EZ.7VF:;GFJ@(N]D9W.HZ[@L+SMNL7WP<S^"
	"QM0#S-D)T=X.U>,2U^44V.85V><6V><6V><6V><6V><6V><6V><6V><6T=L.+3QJ"
	")#!A+3QJ/4AZ/4QZ/4QZ15\"\"3EB+5F\"37FF;9G&C;GFK;GFC9FV+;G63=WF3=X&;"
	"?X&;?X6CAXFCAXFC?X6C?X6C?X6C?X6C?X&;=WV;=WV;=WV;;GF3;G63;G&39&^1"
	"8FJ(66*$2U%V1$UP3E5Y3UI^15Z%,W&:(8Z[%:'1$Z34%YK)&XNY%Y;%$J;6$:C9"
	"$Z35'HV]-6R;26\"06665:'.D=X*SBI7&FZ;8J[;GM<'RO<GZP\\_`R-8%S-D)T-X-"
	"U.(1U^44V.85V><6V><6V><6V><6V><6V><6V><6V><6V><69G&C)#1A+3QJ-41R"
	"/4QZ15\"\"3EB+5F\"37FF;9G&C;GFK=X&T;GFC7FF+9FV+;G&3;GF3=WV;?X&;?X&;"
	"?X6C?X&;?X&;?X&;=WV;=WV;=WF3=WF3;G63;G&3;G&396^+8FJ)76*%359Z04EN"
	"1T]T3U=\\3UE_0U^%,G.;'I\"^%*/3$J;6%IS,&HV\\%IK*$:C9$:G:$J?8(8JZ1EN+"
	"3F:586R=<7RM@HV_E)_1I*_AL;WNN<7VOLO[QM,#R]<(SMP+TM\\/U>(2U^04V.85"
	"V.85V><6V><6V><6V><6V><6V><6V><6V><6J+'E/4AZ-4!R/4AZ15\"\"3EB+5F\"3"
	"7FF;9G&C;GFK=X&T?XFT;GFC7F6\"7FF\"9FF+9FV+;G63=WF3=WV;=WV;=WV;=WV;"
	"=WV;=WF3;GF3;G63;G63;G&39FV+96N+86>(5V\"\"3U=W1DUR1D]T359\\45I^35E]"
	"0U^%+W:?&Y7#$Z35$J;7%9W-&8^^%9O,$:C9$:G:$:G:-6^?36&25FR=:G:G>X:X"
	"C)C)GJG;K+?IML+SO\\K\\P]#`R-4%S=H*S]P,TM`/U>(2U^44U^44V.85V><6V><6"
	"V><6V><6V><6V><6V><6V><6T=L.=X6T3EB+15\"\"3EB+5F\"37FF;9G&C;GFK=X&T"
	"?XF\\?XV\\7FF35EQZ5F\"\"7F6\"7F6+9FV+9G&+;G63=WF3=WF3;G63;G63;G63;G&3"
	"9G&+9FV+96F+8&F*7&6(5EY]3U=Y1TYQ1$MO3E5[55V#5%R\"4%=[0%Z$*7FC%YO*"
	"$:C9$:G:$Z/3&)+!%9O,$:C9$:G:$JC9.G.D5&B997\"A=8\"QAI'\"EZ+4I[/DLK_O"
	"N\\?XPL[_Q]4$R]<'SML+T-X-T^$0U>,2U^44U^44V.85V.85V><6V><6V><6V><6"
	"V><6V><6V><6V>,6N,;U=X6T5F\"35F637FV;9G6C;GVK=X&T?XF\\AY'$=X6T3EAZ"
	"3EAZ5EAZ5EQZ7F\"\"7F6\"9FF+9FV+;G&39G&39FV+9FV+9FV+9FF+7FF+96F*7F6)"
	"5V\"!4EI^2D]S.$!F/D9L2U)W5%V!55V#4UM`35-V0U)W)GNE%9[-$:C9$:G:$Z/3"
	"%Y'!%9O,$:C9$:G:$J?8/7FJ7&Z?;GFK@(N]D9W.H:W>K[OLN,3UOLO[QM,#RM<'"
	"S-H)T-X-TM`/U.(1U>,2UN03U^04V.85V.85V.85V.85V><6V><6V><6V><6V><6"
	"V><6V><6N,+UAY'$9G6C9G6C;GVK=X&T?XV\\AY'$?XV\\7FF+3E1Z3E1R3E1Z3EAZ"
	"5ER\"5F\"\"7F\"\"7F6\"9FF+9FF+7FF+7F6\"7F6+7F6\"7F6\"762\"5%R!3%%U,CIC,SQC"
	"/41J1U!U5%R!66*&55V\"4EI^2U%S1$]Q,&*)%9_.$:G:$:G:$J34&)+\"%IO,$JC9"
	"$:G:&I_066R=8GFJ>(2UB97&G*?8J;7FM<'RO<GZP]#`R=8&S-H)T-X-TM`/U.(1"
	"U.(1UN03U^04UN03U^44V.85V.85V.85V><6V><6V><6V><6V><6V><6V><6V>,6"
	"P<K^F*75?XV\\=X6T?XF\\?XV\\;GFC5F6+3EAZ3E1Z15!R3E1R3E1Z3EAZ5EQZ5ER\""
	"5EQZ7F6\"5F\"\"5F\"\"5ER\"5F\"\"5F\"\"55N!2E%W+C5B+#5>,SMB/$9L25%V5F\"%766+"
	"7&6,66\"$45=Y2E!R/T9J-D%F&H>U$:G:$:G:$J35&93%&)O,%:+3$:G:5FJ<8W.D"
	"<X*SA(_!E)_1HZ_@L;WNNL;WPL[_Q]4$S-H)S]T,TM\\/U.(1U.(1U>,2U>,2UN03"
	"UN03UN03U^44V.85V.85V><6V><6V><6V><6V><6V><6V><6V><6V>,6T=\\.R=,&"
	"F*',;GF;;G&39FF+7F\"\"3EAZ3E1Z15!Z3E1R3E1Z3E1R3EAZ3E1Z5EQZ5EQZ5EAZ"
	"5EQZ15!R-4!J+319)\"Q9)R];*35=,#UC/4AN2E-[5V*'7F>.7VB.762*55I_2E)V"
	"/$EM,SUD*C1>)3UH$:?8$:G:$J76%IW.&YG*&YW.-X2U9&^@7X.T?XJ\\CYK,GJK;"
	"J[?HML/SOLO[QM,#R]D(S]P,T=\\.T^$0U>(2U>,2U>,2UN03UN,3UN03UN03U^44"
	"U^44V.85V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.R=<&L+7E;GFC"
	"14QR-4!A+3AA-4!A-41J-4!J-4!J-4!J-4!A)#!9'\"A1'\"A1%\"11%!Q)%\"!1'\"A1"
	"'\"Q9)#19+3QA-D1J1DUS3UJ\"6&6*8FV28&N.6F.*66&'1T]U/$5K,T%G*C5>)\"]:"
	")3%>'7>G$:G:$J;7$JC9(93%&I_086V>:7RM?(BYBY;(F*/5IK+CLK[OO,CYP]#`"
	"RM<'S]P,TM\\/T]`0U>(2UN03UN,3U>(2U>,2UN03UN03UN03U^44U^44V.85V.85"
	"V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.R=<&R=,&P<K^J+'E9G&;+3QJ"
	"%\"!1%!Q)#!Q)#!A)#!A)#!A!#!1!#!A!#!Q)%\"!)'\"11'\"Q9)#!9+3QA-4!J14QR"
	"3EAZ5F\"\"76B09&Z6:7&79FZ47V:-14QS/49J,CUD)3-='RM8(C!>*39C,$!O$:G:"
	"$Z76$Z?8&YW.1'VO:76F=H&SA9#\"E*#1H:[>K;GJM\\3TP,S]Q]4$S-H)T-X-U.$1"
	"U.(1U>,2U>,2UN03U>,2U>,2UN,3UN03UN03U^04U^44U^44V.85V><6V><6V><6"
	"V><6V><6V><6V><6V>,6V>,6T=\\.T=L.R=,&P<K^N,+UJ+7EH*G=CYW,7FF3-4!J"
	"'\"A1#!Q)#!Q)#!A)%!Q)%\"!1'\"A1)#!9+3AA-4!J/4AR151Z5ER\"7F6+97\"297*9"
	":7\"79&^155N\"1$=Q,SYH(\"Q7%B)2&214'RU:*31C,#UL.$5T'H^_%*76%*;7&:'2"
	"972D:8.T@(R]D)S-GJO;K+CIM<+RO<GZQ=(\"R]@(S]T,TM\\/T^$0U>(2U>,2U>,2"
	"U>,2UN,3U>,2UN03UN03UN03U^44U^44V.85V.85V><6V><6V><6V><6V><6V><6"
	"V><6V>,6V>,6T=\\.T=L.R=<&P<[^N,;UL+KMJ+'=F*75CYG,?XF\\=X&T9G6C152\""
	"-41R+3AA+3AA+3AA-4!J/4AR15!R3EB\"5F\"\"7FF39G&39G&;7F6+15!Z)#1A)#!9"
	"%\"!1%2%2%B)3&\"=4'RU:)3%@+3II-$9U0DU^0EF)%*76$JC9WX.S`!9#@(R]C9G*"
	"FJ;7J;7FM<'RO<KZQ-$!R=8&SML+TM\\/U.$1U.(1U>,2UN,3UN,3UN03UN,3UN03"
	"UN03UN03UN03U^44V.85V.85V.85V><6V><6V><6V><6V><6V><6V><6V><6V>,6"
	"V>,6T=\\.R=<&R=,&P<K^L+[MJ+7EH*W=F*'5AY7$?XF\\;GVK9G6C5F633ER+15\"\""
	"-41R+3QJ+3AJ)#!A'\"Q9'\"Q9'\"A9'\"A9'\"A9'\"A9'\"A9'\"A9'\"A9'\"A9'\"E9'2U:"
	"(\"Y<*#-E*CQI-3]Q.$IY1%&!0U:7`!9#WX.S`!9#@8V^D)O-FZ?8J;7FL[_PO,CY"
	"P]#`R=<&S=L*T=\\.T]`0U.(1U.(1UN,3UN,3UN03UN03UN03UN03UN03U^44U^44"
	"U^44V.85V.85V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=L."
	"R=<&P<[^N,;UL+[MJ+7EH*G=F*',AY7$?XV\\=X&T9G6C7FF;3ER+152\"/4QZ/41R"
	"-4!R+3QJ+3AJ+3AJ)#1A)#1A)#1A)#1A)#1A)#1A)#1A+3AJ+CEK+CUK-D%S.$=U"
	"04Q^0U.#36\".7FB;`!9#WX.S`!9#E:#-HZ_;K;CGM,#QN,?YP\\_`R-4%R]D(SMP+"
	"T-X-T]`0UN,3U.$1T]`0T^$0U.$1U.(1U^04U>(2T^$0U.$1U.(1U>,2U^04UN03"
	"UN03UN03UN03V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=L.R=,&P<[^"
	"N,;UL+[MJ+7EH*G=F*'5CYG,?XV\\=X&T;GFK7FV;5F633ER+3EB+15\"\"/4QZ/4AZ"
	"/4AZ/4AZ/4AZ/4AZ-41R-41R/4AZ/4AZ/4AZ/4QZ1E&#1U:$45N&5&*17VF<9W6D"
	"=8*R@I\"^`!9#`!9#K;G2M<'>N\\?LN<KXQ=(\"QM0#QM,#Q=(\"Q=(\"R=<&T-T-TM\\/"
	"S]P,RM@'R-4$R]D(T-T-T^$0T=\\.R]D(R=8%S-H)T-T,T=T.S]T,R]D(R]D(S]T,"
	"V><6V><6V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=\\.R=<&R=,&P<[^N,;UL+[M"
	"J+7EH*W=F*75CYW,AY'$?XF\\;GVK;GFK9G&C7FV;7F635F\"33UR,3UV,3UV,3UV,"
	"3UF,3UJ,3UF,3ER+3ER+5F\"36&.566>686R>8W&@;GRK>H6W?9\"`CYO*G*G-`!9#"
	"M<C!R-.^RM3-NL?7P\\_SQ]4\"Q-(!N\\?WK[OLK;CIML+RQ-$!R=8%P<W^M,'PK;GI"
	"ML'RPL[^R-8%P\\__ML+RKKKJM<+QP,W\\Q,_`O<GYL[_OL[_OOLKZV><6V><6V><6"
	"V><6V><6V><6V><6V><6V>,6V>,6T=\\.T=L.R=<&R=,&P<[^N,;UN,+UL+KMJ+'E"
	"H*G=F*'5CYG,AY'$?XF\\=X6T=X&T;GFK;WJL9W:D:'.E9W*D:'.E9W.D9G&C9W*D"
	":':E9W6D;WJL;WNL<7^N>X6X@HV_B)7%DIW/F*77I+#:IK7-K[RMU>\">X.F-P<N("
	"O,>WQ,_GQM/\\OLGZIK+BC)C'A9'!F*34LK[NO,CYK;KIDIW-@HV^DI[.KKKJN\\?W"
	"KKKJDI[.@HZ^D9W,JK;FLKWNGZO;BY?&BY?&HJ[>V><6V><6V><6V><6V><6V><6"
	"V><6V><6V>,6V>,6V>,6T=\\.T=L.R=<&R=,&P<[^P<K^N,;UL+[ML+KMJ+'EH*G="
	"F*'5CIS+CIC+AI3#AY'$?HR[?XF\\?XJ\\@(J]?XF\\?XF\\?XJ\\@(F]?XR\\@8V^AI+#"
	"AY'$BYC(DIW/F:/3GJG;HZ_@J[;HL;SDM+[-NL29U-QIZ?%9U-UIP,J?Q,_>QM+Y"
	"M\\+SE)_/;WJI9&^>?XJZI*_@L[_OG*G8='^O7FF9='^OG:G9LK[NGJK9=8\"O7VJ9"
	"='^NF:74HZ_?B93$:G6E:G6EBY?&V><6V><6V><6V><6V><6V><6V><6V><6V><6"
	"V>,6V>,6V>,6T=\\.T=\\.T=L.R=<&R=,&P<[^P<K^N,;UL+[MK[GLI[3DIJ_CGJO;"
	"G:;:E:+2EI_3EI_3CYS,D9W.D)S-D9[.D)W-E)_1E)_1EJ'3EZ/4G*?9GZK<I*_A"
	"J;7FK+GILKWOML+SN<7LNL74O<>;U-QIZ?%9U=UJPLR@Q=+?QM+YM\\/TE*#0;WJI"
	"9&^>?XJZI*_@L[_OG:G9='^O7FF9='^OG:G8L;[NGJK9=8\"O7VJ9='^NF:34H[#?"
	"B93$:G6E:G6EBY?&V><6V><6V><6V><6V><6V><6V><6V><6V><6V><6V>,6V>,6"
	"V>,6V>,6T=\\.T=\\.T=L.R=<&R=,&P<[^P,G]M\\7TML#SK;OJK+GIJK7GH[#@HZ_@"
	"HJS?HZW@HZ_@HZ[@HZW@I*[AI*_AI+#AI;#BJ;3FJK;GK;CJLKWOM<#RN,3UN\\;X"
	"P,O]PL[XP,SDO<BXP\\V*R]1XQ,Z+PLV]RM7MR]@!P<W]J+/DC9C(AI'!F*34LK[N"
	"O<GZKKKJDIW-@HV]DIW-KKGJO,CXKKOJDY_.@X^^DIW-J[?GLK[NGZO;BY?&BY?&"
	"HJ[>V><6U^44UN03UN03UN03U^44V><6U^44UN03UN03UN03U^44V>,6V.(5UM`3"
	"SMP+SMP+T-H,T=L-R-8%QL`#Q,X!O,GYNL?WN</VL+[MK;KJK;KJK;?JK;CJK[KL"
	"K[KLKKGKK[GLK[KLL+SMLK[OM+_QM<#RM<'RN<7UO,?YO\\O\\P\\[`P]#`Q='_Q-#T"
	"PLW=P,O#P<RVQ,['R=3DS=C]S]P)R=8%OLKZLKWNK;GIM<+QPL[_Q]0$P<W^M<'Q"
	"K;CIL[_PP<S]Q],#PL__ML+RKKKJM,#PO\\O[P<[^N\\?WLK_NL[_OOLKZU^44TM`."
	"S=L*RM@'S=L*TM`.U>,2TM`.S=L*RM@'S=L*TM`.U>,2T]T0S=@*RM4'R]8(T=L-"
	"S-D(RM<%Q<\\!P,K]N\\CWO<GZO<?ZN\\3XM+WQKKGJK;GJL+SMM<#RM+_QL;WMK[OK"
	"L;WMM\\+TNL7WN\\?XN,3UML+SM\\+SN\\?WP<W]QM,#Q],$Q-$!P,SYO\\OOQ-#LR-/L"
	"R]?TRM;[R=4\"Q]0$Q]0$Q]4$Q=,\"PL[_P<W]P\\_`Q]0$R=8&R-4%P\\_`PL[^Q-(!"
	"Q]4$RM@&R=8%QM,\"Q-(!P]#`Q=,\"QM,\"Q]4$RM@'S]T,T=\\.Q=,\"ML/SKKOJML/S"
	"P]#`RM@&P]#`ML/SKKOJML/SQ-(!R]D(Q]0#N<;VL+KLM+WPP,K]Q]$#PLO^K[SK"
	"J+/DK;GJN,3UO<CZN,+UK+?IHZ[?HJW>J+3DL+SMKKKJI;'AGZO;I[+CL[[PO<CZ"
	"O<GZML+RJ;7EI+#@K+CHN<7VP\\_`P<W^M\\/TK;GIK[OHN<7RQ='\\R=4!P<S[M<'Q"
	"K+GHM,#QO\\S\\Q=(\"O\\O\\L[[OK;CIM<'QPLW^R-0%Q,\\!M\\+TK[OKM,#PO\\S\\R-4$"
	"QM0#N\\CWLK[NLK_NO,CXR=8%T=\\.U>,2RM@&LK[NDY_.@X^^DY_.KKOJO,GXKKOJ"
	"DY_.@X^^DY_.K[SLO\\O[M,'PFJ;6AY/\"CIK*I[/CM\\#SK+;HDIS-@HR]D)O,JK7F"
	"N,/TK;GJF*/4A(^_A(^_EJ'1IK'BH:S=BY;&?8FYC)?'J+/DNL;WO,?XJ;7EC9C)"
	"?XNZCIG*J;7ENL;WM<'QGZK;B97$BI7%H:S<NL;VP,S\\KKKJDI[-@HZ]DIW-K;GI"
	"N\\;WK;GIDIW-@HZ]DI[-K[KKOLOZL\\#PFJ;6AY/\"CIK*J+3DN\\CWN,3TH:W=BY?&"
	"BY?&HJ[>OLKZS]T,UN03Q-(!H:W==8\"O7VJ9=8\"OGJK9LK_NGJK9=8\"O7VJ9=8\"O"
	"GJK:M<'QIK+A@(N[9'\"?;WNJDY_/JK?FFJ?6='^O7FJ9='^OG*;8L[WOJ+/DAY+#"
	":'.C:'.CA9\"`GJG:E)_0<7VL7FF8<WZNG*C8NL7VNL7VG:G9<WZN7FF8<WZNG*?7"
	"M<#QK+?HBI7%:G6D:G6DBI;&KKKJN,3TGZO:=8\"O7FJ9=(\"OG:G9L;WNG:G9=(\"O"
	"7FF9=(\"OGJK:M<#QIK+A@(N[9'\"?;WNJE*#0L+SLJ[CGBY?&:G6E:G6EBY?&L[_O"
	"R]D(UN03Q-(!H:W==8\"O7VJ9=8\"OGJK9LK_NGJK9=8\"O7VJ9=8\"OGJK:M<'QIK+A"
	"@(N[9'\"?;WNJDY_/JK?FFJ?6='^O7FJ9=(\"OG:K9M<+RJ[?GB97$:72D:72CAI+!"
	"H*O<EJ'1<GZM7FF8='^NGZO;O,GXO<GYGZO;='^O7FF8='^OG:G9ML+RK;GIBI;&"
	":G6D:G6DBI;&KKKJN,7UGZO;=8\"O7VJ9=8\"OGJK9LK[NGJG9=8\"O7VJ9=8\"OGJK:"
	"M<'QIK+A@(N[9'\"?;WNJE*#0L+SLJ[CGBY?&:G6E:G6EBY?&L[_OR]D(UN03RM@&"
	"LK[NDY_.@X^^DY_.KKOJO,GXKKOJDY_.@X^^DY_.K[SLO\\O[M,'PFJ;6AY/\"CIK*"
	"I[/CM\\/SK+GHDI[-@HZ^DI[.K[OKO\\O\\N<7UH:S<BI7%BI7%G:G9K[OLJ;3ED)S,"
	"@HZ]DI[-L+WLQ=(\"Q=,\"L;WMDI[.@HZ^DI[-KKKJO\\S\\N<;UH:W<BY?&BY?&HJ[>"
	"N\\?WP<W]K[SLDY_.@X^^DY_.KKOJO,GXKKOJDY[.@X^^DY_.K[SLO\\O[M,'PFJ;6"
	"AY/\"CIK*J+3DN\\CWN,3TH:W=BY?&BY?&HJ[>OLKZS]T,UN03T=\\.Q=,\"ML/SKKOJ"
	"ML/SP]#`RM@&P]#`ML/SKKOJML/SQ-(!R]D(Q]0#N<;VL+SLM,#PP,W]Q]0#PL__"
	"ML+RKKOJML/SP]#`S-H)R=<&O<GYL;[ML;WMN\\CWP]#_O\\S\\M,'QK;GIM<'QP]#`"
	"S]P+T-T,Q-(!ML+RKKKJML+RP]#`S-H)R=<&O<GYL[_OL[_OOLKZRM@&S=L*Q-(!"
	"ML/SKKOJML/SP]#`RM@&P]#`ML/SKKOJML/SQ-(!R]D(Q]0#N<;VL+SLM,#PP,W]"
	"RM@&R=8%O<KZL[_OL[_OOLKZR]D(U.(1V.85";