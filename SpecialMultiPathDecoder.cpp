F_function_vectorized(LLR[0][11].data(), LLR[0][10].data(), 1024);
F_function_vectorized(LLR[0][10].data(), LLR[0][9].data(), 512);
F_function_vectorized(LLR[0][9].data(), LLR[0][8].data(), 256);
Repetition_multiPath(8, 0);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), Bits[currentPath].data()+0, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
Rate0_multiPath(6, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
Rate0_multiPath(5, 320);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 352);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 368);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+368, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 376);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+376, 4);
SPC_multiPath(2, 380);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+376, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+368, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+352, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+320, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+256, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+256, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
Repetition_multiPath(5, 384);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+384, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 416);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+416, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 432);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+432, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 440);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+440, 4);
SPC_multiPath(2, 444);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+440, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+432, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+416, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+384, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 448);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+448, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 464);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+464, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 472);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 474);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+472, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+472, 4);
Rate1_multiPath(2, 476);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+472, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+464, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+448, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+448, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 480);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
SPC_multiPath(2, 484);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+480, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+480, 8);
SPC_multiPath(3, 488);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+480, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+480, 16);
SPC_multiPath(4, 496);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+480, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+448, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+256, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][10].data(), LLR[currentPath][9].data(), Bits[currentPath].data()+0, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
Rate0_multiPath(6, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
Repetition_multiPath(5, 576);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+576, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 608);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+608, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 624);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+624, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 632);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+632, 4);
Rate1_multiPath(2, 636);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+632, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+624, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+608, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+576, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+512, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+512, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
Repetition_multiPath(5, 640);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+640, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Rate0_multiPath(3, 672);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 680);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+680, 4);
SPC_multiPath(2, 684);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+680, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+672, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+672, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 688);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+688, 4);
SPC_multiPath(2, 692);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+688, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+688, 8);
SPC_multiPath(3, 696);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+688, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+672, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+640, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 704);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+704, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 712);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+712, 4);
SPC_multiPath(2, 716);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+712, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+704, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+704, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 720);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+720, 4);
SPC_multiPath(2, 724);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+720, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+720, 8);
SPC_multiPath(3, 728);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+720, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+704, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+704, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 736);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+736, 4);
SPC_multiPath(2, 740);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+736, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+736, 8);
SPC_multiPath(3, 744);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+736, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+736, 16);
SPC_multiPath(4, 752);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+736, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+704, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), Bits[currentPath].data()+512, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 768);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 784);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+784, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 792);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+792, 4);
SPC_multiPath(2, 796);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+792, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+784, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+768, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+768, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 800);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+800, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 808);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+808, 4);
SPC_multiPath(2, 812);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+808, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+800, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+800, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 816);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+816, 4);
SPC_multiPath(2, 820);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+816, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+816, 8);
SPC_multiPath(3, 824);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+816, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+800, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+768, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 832);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+832, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 840);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+840, 4);
SPC_multiPath(2, 844);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+840, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+832, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+832, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 848);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+848, 4);
SPC_multiPath(2, 852);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+848, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+848, 8);
SPC_multiPath(3, 856);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+848, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+832, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+832, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 864);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+864, 4);
SPC_multiPath(2, 868);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+864, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+864, 8);
SPC_multiPath(3, 872);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+864, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+864, 16);
SPC_multiPath(4, 880);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+864, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+832, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+768, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 896);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+896, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 904);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+904, 4);
SPC_multiPath(2, 908);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+904, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+896, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 912);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+912, 4);
SPC_multiPath(2, 916);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+912, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+912, 8);
SPC_multiPath(3, 920);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+912, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+896, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 928);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+928, 4);
Rate1_multiPath(2, 932);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+928, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+928, 8);
Rate1_multiPath(3, 936);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+928, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+928, 16);
Rate1_multiPath(4, 944);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+928, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+896, 64);
SPC_multiPath(6, 960);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][11].data(), LLR[currentPath][10].data(), Bits[currentPath].data()+0, 1024);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][10].data(), LLR[currentPath][9].data(), 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
Repetition_multiPath(6, 1024);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1024, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 1088);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Rate0_multiPath(3, 1104);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1112);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1112, 4);
SPC_multiPath(2, 1116);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1112, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1104, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1088, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1088, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 1120);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1120, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1128, 4);
SPC_multiPath(2, 1132);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1128, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1120, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1120, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1136);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1136, 4);
SPC_multiPath(2, 1140);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1136, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1136, 8);
SPC_multiPath(3, 1144);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1136, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1120, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1088, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1024, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+1024, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 1152);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1152, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 1168);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1168, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1176);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1176, 4);
SPC_multiPath(2, 1180);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1176, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1168, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1152, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1152, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 1184);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1184, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1192);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1192, 4);
SPC_multiPath(2, 1196);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1192, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1184, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1184, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1200);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1200, 4);
SPC_multiPath(2, 1204);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1200, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1200, 8);
SPC_multiPath(3, 1208);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1200, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1184, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1152, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1152, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 1216);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1216, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1224);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1224, 4);
SPC_multiPath(2, 1228);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1224, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1216, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1216, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1232);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1232, 4);
SPC_multiPath(2, 1236);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1232, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1232, 8);
SPC_multiPath(3, 1240);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1232, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1216, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1216, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1248);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1248, 4);
Rate1_multiPath(2, 1252);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1248, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1248, 8);
Rate1_multiPath(3, 1256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1248, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1248, 16);
Rate1_multiPath(4, 1264);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1248, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1216, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1152, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1024, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), Bits[currentPath].data()+1024, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 1280);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1280, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 1296);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1296, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1304);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1304, 4);
SPC_multiPath(2, 1308);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1304, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1296, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1280, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1280, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 1312);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1312, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1320);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1320, 4);
Rate1_multiPath(2, 1324);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1320, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1312, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1312, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 1328);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 1330);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1328, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1328, 4);
Rate1_multiPath(2, 1332);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1328, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1328, 8);
Rate1_multiPath(3, 1336);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1328, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1312, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1280, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1280, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 1344);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 1348);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 1350);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1348, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1344, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1344, 8);
SPC_multiPath(3, 1352);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1344, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1344, 16);
SPC_multiPath(4, 1360);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1344, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1344, 32);
SPC_multiPath(5, 1376);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1344, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1280, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+1280, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1408);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1408, 4);
SPC_multiPath(2, 1412);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1408, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1408, 8);
SPC_multiPath(3, 1416);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1408, 16);
SPC_multiPath(4, 1424);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1408, 32);
SPC_multiPath(5, 1440);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1408, 64);
SPC_multiPath(6, 1472);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1280, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1024, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][10].data(), LLR[currentPath][9].data(), Bits[currentPath].data()+1024, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 1536);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1536, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 1552);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 1556);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 1558);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1556, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+1552, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1552, 8);
SPC_multiPath(3, 1560);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1552, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1536, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1536, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1568);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1568, 4);
SPC_multiPath(2, 1572);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1568, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1568, 8);
SPC_multiPath(3, 1576);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1568, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1568, 16);
SPC_multiPath(4, 1584);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1568, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1536, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1536, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1600);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1600, 4);
SPC_multiPath(2, 1604);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1600, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1600, 8);
SPC_multiPath(3, 1608);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1600, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1600, 16);
SPC_multiPath(4, 1616);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1600, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1600, 32);
SPC_multiPath(5, 1632);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1600, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1536, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+1536, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1664);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1664, 4);
SPC_multiPath(2, 1668);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1664, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1664, 8);
SPC_multiPath(3, 1672);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1664, 16);
SPC_multiPath(4, 1680);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1664, 32);
SPC_multiPath(5, 1696);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1664, 64);
SPC_multiPath(6, 1728);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1536, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), Bits[currentPath].data()+1536, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 1792);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1792, 4);
SPC_multiPath(2, 1796);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1792, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1792, 8);
SPC_multiPath(3, 1800);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1792, 16);
SPC_multiPath(4, 1808);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1792, 32);
SPC_multiPath(5, 1824);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1792, 64);
SPC_multiPath(6, 1856);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+1792, 128);
SPC_multiPath(7, 1920);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1536, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1024, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 1024);
