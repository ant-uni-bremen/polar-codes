F_function_vectorized(LLR[0][7].data(), LLR[0][6].data(), 64);
F_function_vectorized(LLR[0][6].data(), LLR[0][5].data(), 32);
F_function_vectorized(LLR[0][5].data(), LLR[0][4].data(), 16);
Rate0_multiPath(4, 0);
G_function_0R_vectorized(LLR[0][5].data(), LLR[0][4].data(), 16);
F_function_vectorized(LLR[0][4].data(), LLR[0][3].data(), 8);
Rate0_multiPath(3, 16);
G_function_0R_vectorized(LLR[0][4].data(), LLR[0][3].data(), 8);
F_function(LLR[0][3].data(), LLR[0][2].data(), 4);
Repetition_multiPath(2, 24);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+24, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 28);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+28, 2);
Rate1_multiPath(1, 30);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+28, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+24, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+16, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+0, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+0, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+32, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 40);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+40, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 44);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+44, 2);
Rate1_multiPath(1, 46);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+44, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+40, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+32, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+32, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 48);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+48, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 52);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+52, 2);
Rate1_multiPath(1, 54);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+52, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+48, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+48, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 56);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+56, 2);
Rate1_multiPath(1, 58);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+56, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+56, 4);
Rate1_multiPath(2, 60);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+56, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+48, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+32, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+0, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+64, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 72);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+72, 4);
Rate1_multiPath(2, 76);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+72, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+64, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+64, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 80);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+80, 2);
Rate1_multiPath(1, 82);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+80, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+80, 4);
Rate1_multiPath(2, 84);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+80, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+80, 8);
Rate1_multiPath(3, 88);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+80, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+64, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+64, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 96);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+96, 2);
Rate1_multiPath(1, 98);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+96, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+96, 4);
Rate1_multiPath(2, 100);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+96, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+96, 8);
Rate1_multiPath(3, 104);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+96, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+96, 16);
Rate1_multiPath(4, 112);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+96, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+64, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 64);
