F_function_vectorized(initialLLR.data(), LLR[0][10].data(), 1024);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 380);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+380, 2);
Rate1_multiPath(1, 382);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+380, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 444);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+444, 2);
Rate1_multiPath(1, 446);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+444, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 484);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+484, 2);
Rate1_multiPath(1, 486);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+484, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+480, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+480, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 488);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+488, 2);
Rate1_multiPath(1, 490);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+488, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+488, 4);
Rate1_multiPath(2, 492);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+488, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+480, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+480, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 496);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+496, 2);
Rate1_multiPath(1, 498);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+496, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+496, 4);
Rate1_multiPath(2, 500);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+496, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+496, 8);
Rate1_multiPath(3, 504);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+496, 8);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 684);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+684, 2);
Rate1_multiPath(1, 686);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+684, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 692);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+692, 2);
Rate1_multiPath(1, 694);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+692, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+688, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+688, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 696);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+696, 2);
Rate1_multiPath(1, 698);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+696, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+696, 4);
Rate1_multiPath(2, 700);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+696, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 716);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+716, 2);
Rate1_multiPath(1, 718);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+716, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 724);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+724, 2);
Rate1_multiPath(1, 726);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+724, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+720, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+720, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 728);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+728, 2);
Rate1_multiPath(1, 730);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+728, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+728, 4);
Rate1_multiPath(2, 732);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+728, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 740);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+740, 2);
Rate1_multiPath(1, 742);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+740, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+736, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+736, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 744);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+744, 2);
Rate1_multiPath(1, 746);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+744, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+744, 4);
Rate1_multiPath(2, 748);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+744, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+736, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+736, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 752);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+752, 2);
Rate1_multiPath(1, 754);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+752, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+752, 4);
Rate1_multiPath(2, 756);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+752, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+752, 8);
Rate1_multiPath(3, 760);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+752, 8);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 796);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+796, 2);
Rate1_multiPath(1, 798);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+796, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 812);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+812, 2);
Rate1_multiPath(1, 814);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+812, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 820);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+820, 2);
Rate1_multiPath(1, 822);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+820, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+816, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+816, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 824);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+824, 2);
Rate1_multiPath(1, 826);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+824, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+824, 4);
Rate1_multiPath(2, 828);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+824, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 844);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+844, 2);
Rate1_multiPath(1, 846);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+844, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 852);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+852, 2);
Rate1_multiPath(1, 854);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+852, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+848, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+848, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 856);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+856, 2);
Rate1_multiPath(1, 858);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+856, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+856, 4);
Rate1_multiPath(2, 860);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+856, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 868);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+868, 2);
Rate1_multiPath(1, 870);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+868, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+864, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+864, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 872);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+872, 2);
Rate1_multiPath(1, 874);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+872, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+872, 4);
Rate1_multiPath(2, 876);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+872, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+864, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+864, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 880);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+880, 2);
Rate1_multiPath(1, 882);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+880, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+880, 4);
Rate1_multiPath(2, 884);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+880, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+880, 8);
Rate1_multiPath(3, 888);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+880, 8);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 908);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+908, 2);
Rate1_multiPath(1, 910);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+908, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 916);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+916, 2);
Rate1_multiPath(1, 918);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+916, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+912, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+912, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 920);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+920, 2);
Rate1_multiPath(1, 922);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+920, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+920, 4);
Rate1_multiPath(2, 924);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+920, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 960);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+960, 2);
Rate1_multiPath(1, 962);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+960, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+960, 4);
Rate1_multiPath(2, 964);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+960, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+960, 8);
Rate1_multiPath(3, 968);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+960, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+960, 16);
Rate1_multiPath(4, 976);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+960, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+960, 32);
Rate1_multiPath(5, 992);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+960, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(initialLLR.data(), LLR[currentPath][10].data(), Bits[currentPath].data()+0, 1024);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1116);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1116, 2);
Rate1_multiPath(1, 1118);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1116, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1132);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1132, 2);
Rate1_multiPath(1, 1134);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1132, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1140);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1140, 2);
Rate1_multiPath(1, 1142);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1140, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1136, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1136, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1144);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1144, 2);
Rate1_multiPath(1, 1146);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1144, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1144, 4);
Rate1_multiPath(2, 1148);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1144, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1180);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1180, 2);
Rate1_multiPath(1, 1182);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1180, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1196);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1196, 2);
Rate1_multiPath(1, 1198);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1196, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1204);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1204, 2);
Rate1_multiPath(1, 1206);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1204, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1200, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1200, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1208);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1208, 2);
Rate1_multiPath(1, 1210);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1208, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1208, 4);
Rate1_multiPath(2, 1212);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1208, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1228);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1228, 2);
Rate1_multiPath(1, 1230);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1228, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1236);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1236, 2);
Rate1_multiPath(1, 1238);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1236, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1232, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1232, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1240);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1240, 2);
Rate1_multiPath(1, 1242);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1240, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1240, 4);
Rate1_multiPath(2, 1244);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1240, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1308);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1308, 2);
Rate1_multiPath(1, 1310);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1308, 2);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1352);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1352, 2);
Rate1_multiPath(1, 1354);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1352, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1352, 4);
Rate1_multiPath(2, 1356);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1352, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1344, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1344, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1360);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1360, 2);
Rate1_multiPath(1, 1362);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1360, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1360, 4);
Rate1_multiPath(2, 1364);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1360, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1360, 8);
Rate1_multiPath(3, 1368);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1360, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1344, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1344, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1376);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1376, 2);
Rate1_multiPath(1, 1378);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1376, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1376, 4);
Rate1_multiPath(2, 1380);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1376, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1376, 8);
Rate1_multiPath(3, 1384);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1376, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1376, 16);
Rate1_multiPath(4, 1392);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1376, 16);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1412);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1412, 2);
Rate1_multiPath(1, 1414);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1412, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1408, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1408, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1416);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1416, 2);
Rate1_multiPath(1, 1418);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1416, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1416, 4);
Rate1_multiPath(2, 1420);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1416, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1408, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1424);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1424, 2);
Rate1_multiPath(1, 1426);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1424, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1424, 4);
Rate1_multiPath(2, 1428);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1424, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1424, 8);
Rate1_multiPath(3, 1432);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1424, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1408, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1440);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1440, 2);
Rate1_multiPath(1, 1442);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1440, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1440, 4);
Rate1_multiPath(2, 1444);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1440, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1440, 8);
Rate1_multiPath(3, 1448);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1440, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1440, 16);
Rate1_multiPath(4, 1456);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1440, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1408, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1408, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1472);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1472, 2);
Rate1_multiPath(1, 1474);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1472, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1472, 4);
Rate1_multiPath(2, 1476);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1472, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1472, 8);
Rate1_multiPath(3, 1480);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1472, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1472, 16);
Rate1_multiPath(4, 1488);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1472, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1472, 32);
Rate1_multiPath(5, 1504);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1472, 32);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1560);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1560, 2);
Rate1_multiPath(1, 1562);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1560, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1560, 4);
Rate1_multiPath(2, 1564);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1560, 4);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1572);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1572, 2);
Rate1_multiPath(1, 1574);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1572, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1568, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1568, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1576);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1576, 2);
Rate1_multiPath(1, 1578);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1576, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1576, 4);
Rate1_multiPath(2, 1580);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1576, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1568, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1568, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1584);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1584, 2);
Rate1_multiPath(1, 1586);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1584, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1584, 4);
Rate1_multiPath(2, 1588);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1584, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1584, 8);
Rate1_multiPath(3, 1592);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1584, 8);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1604);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1604, 2);
Rate1_multiPath(1, 1606);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1604, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1600, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1600, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1608);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1608, 2);
Rate1_multiPath(1, 1610);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1608, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1608, 4);
Rate1_multiPath(2, 1612);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1608, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1600, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1600, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1616);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1616, 2);
Rate1_multiPath(1, 1618);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1616, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1616, 4);
Rate1_multiPath(2, 1620);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1616, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1616, 8);
Rate1_multiPath(3, 1624);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1616, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1600, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1600, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1632);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1632, 2);
Rate1_multiPath(1, 1634);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1632, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1632, 4);
Rate1_multiPath(2, 1636);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1632, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1632, 8);
Rate1_multiPath(3, 1640);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1632, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1632, 16);
Rate1_multiPath(4, 1648);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1632, 16);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1668);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1668, 2);
Rate1_multiPath(1, 1670);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1668, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1664, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1664, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1672);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1672, 2);
Rate1_multiPath(1, 1674);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1672, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1672, 4);
Rate1_multiPath(2, 1676);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1672, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1664, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1680);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1680, 2);
Rate1_multiPath(1, 1682);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1680, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1680, 4);
Rate1_multiPath(2, 1684);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1680, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1680, 8);
Rate1_multiPath(3, 1688);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1680, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1664, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1696);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1696, 2);
Rate1_multiPath(1, 1698);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1696, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1696, 4);
Rate1_multiPath(2, 1700);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1696, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1696, 8);
Rate1_multiPath(3, 1704);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1696, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1696, 16);
Rate1_multiPath(4, 1712);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1696, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1664, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1664, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1728);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1728, 2);
Rate1_multiPath(1, 1730);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1728, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1728, 4);
Rate1_multiPath(2, 1732);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1728, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1728, 8);
Rate1_multiPath(3, 1736);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1728, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1728, 16);
Rate1_multiPath(4, 1744);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1728, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1728, 32);
Rate1_multiPath(5, 1760);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1728, 32);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1796);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1796, 2);
Rate1_multiPath(1, 1798);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1796, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1792, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1792, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1800);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1800, 2);
Rate1_multiPath(1, 1802);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1800, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1800, 4);
Rate1_multiPath(2, 1804);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1800, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1792, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1808);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1808, 2);
Rate1_multiPath(1, 1810);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1808, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1808, 4);
Rate1_multiPath(2, 1812);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1808, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1808, 8);
Rate1_multiPath(3, 1816);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1808, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1792, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1824);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1824, 2);
Rate1_multiPath(1, 1826);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1824, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1824, 4);
Rate1_multiPath(2, 1828);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1824, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1824, 8);
Rate1_multiPath(3, 1832);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1824, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1824, 16);
Rate1_multiPath(4, 1840);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1824, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1792, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1856);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1856, 2);
Rate1_multiPath(1, 1858);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1856, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1856, 4);
Rate1_multiPath(2, 1860);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1856, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1856, 8);
Rate1_multiPath(3, 1864);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1856, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1856, 16);
Rate1_multiPath(4, 1872);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1856, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1856, 32);
Rate1_multiPath(5, 1888);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1856, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+1792, 128);
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
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 1920);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+1920, 2);
Rate1_multiPath(1, 1922);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1920, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+1920, 4);
Rate1_multiPath(2, 1924);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+1920, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+1920, 8);
Rate1_multiPath(3, 1928);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1920, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+1920, 16);
Rate1_multiPath(4, 1936);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1920, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+1920, 32);
Rate1_multiPath(5, 1952);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1920, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+1920, 64);
Rate1_multiPath(6, 1984);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1920, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1792, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1536, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+1024, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 1024);
