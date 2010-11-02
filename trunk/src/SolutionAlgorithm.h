#ifndef SOLUTIONALGORITHM_H
#define SOLUTIONALGORITHM_H

class SolutionAlgorithm {
public:
	// Bus dwells are index, content is the assigned platform
	// return true if a solution was found
	virtual bool solveX() = 0;
	virtual bool solution(int *&gates) const = 0;
	double elapsedTime() const {
		return (_clock_end - _clock_begin) / (double)(CLOCKS_PER_SEC / 1000);
	}
	virtual bool solved() { return _solved; }
	double objectiveFunction() const { return _objectiveFunction; }
protected:
	bool _solved;
	clock_t _clock_begin;
	clock_t _clock_end;
	double _objectiveFunction;
};

#endif /* SOLUTIONALGORITHM_H */
