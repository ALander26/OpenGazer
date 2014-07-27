#include <iostream>

#include "BlinkDetector.h"
#include "EyeExtractor.h"
#include "utils.h"

StateNode::StateNode(int minDuration, int maxDuration, double threshold, double finishThreshold):
	minDuration(minDuration),
	maxDuration(maxDuration),
	threshold(threshold),
	finishThreshold(finishThreshold)
{
}

bool StateNode::isSatisfied(double value) const {
	return threshold > 0 ? value >= threshold : value <= -threshold;
}

bool StateNode::isFinished(double value) const {
	return threshold > 0 ? value < finishThreshold : value > -finishThreshold;
}

LinearStateSystem::LinearStateSystem(const vector<StateNode> &states):
	_states(states),
	_currentState(0),
	_duration(0)
{
	assert(!states.empty());
}

void LinearStateSystem::updateState(double value) {
	//cout << "update state: " << value << endl;
	_duration++;
	if (_currentState > 0 && ((_duration < _states[_currentState].minDuration && !_states[_currentState].isSatisfied(value)) || _duration > _states[_currentState].maxDuration)) {
		setState(0);
	}

	if (!isFinalState() && _states[_currentState + 1].isSatisfied(value)) {
		setState(_currentState + 1);
	}
}

void LinearStateSystem::setState(int state) {
	_currentState = state;
	_duration = 0;
}

int LinearStateSystem::getState() {
	return _currentState;
}

bool LinearStateSystem::isFinalState() {
	return _currentState == (int)_states.size() - 1;
}

LambdaAccumulator::LambdaAccumulator(double lambda, double initial):
	_lambda(lambda),
	_current(initial)
{
}

void LambdaAccumulator::update(double value) {
	_current = (1 - _lambda) * _current + _lambda * value;
}

double LambdaAccumulator::getValue() {
	return _current;
}

BlinkDetector::BlinkDetector():
	_averageEye(cvCreateImage(EyeExtractor::eyeSize, IPL_DEPTH_32F, 1)),
	_accumulator(0.1, 1000.0),
	_states(constructStates()),
	_initialized(false)
{
}

void BlinkDetector::update(const boost::scoped_ptr<IplImage> &eyeFloat) {
	if (!_initialized) {
		cvCopy(eyeFloat.get(), _averageEye.get());
		_initialized = true;
	}

	double distance = cvNorm(eyeFloat.get(), _averageEye.get(), CV_L2);
	_accumulator.update(distance);
	//cout << "update distance" << distance << " -> " << accumulator.getValue() << endl;
	_states.updateState(distance / _accumulator.getValue());
	cvRunningAvg(eyeFloat.get(), _averageEye.get(), 0.05);
}

int BlinkDetector::getState() {
	return _states.getState();
}

vector<StateNode> BlinkDetector::constructStates() {
	vector<StateNode> states;

	// 1.60 - 1.20
	states.push_back(StateNode(0, 0, +0.00, +0.00));
	states.push_back(StateNode(1, 8, +1.80, 0.30));
	states.push_back(StateNode(3, 6, -1.40, -0.20)); // blink
	states.push_back(StateNode(1, 8, +1.80, 0.30));
	states.push_back(StateNode(3, 6, -1.40, -0.20)); // double blink

	return states;
}

