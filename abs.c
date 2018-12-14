int distance(int a, int b){
	if(a == b) return 0;
	else if(a < b) return b - a;
	else return a - b;
}