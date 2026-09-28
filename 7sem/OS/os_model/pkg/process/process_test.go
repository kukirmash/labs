package process

import "testing"

func TestGenerateNextCommandEnd(t *testing.T) {
	p := PSW{PC: 5, TotalCommands: 5, IOProbability: 1.0}

	cmd := p.GenerateNextCommand()
	if cmd.Code != OpEnd {
		t.Fatalf("code = %v, want %v", cmd.Code, OpEnd)
	}
}

func TestGenerateNextCommandIO(t *testing.T) {
	p := PSW{PC: 0, TotalCommands: 10, IOProbability: 1.0}

	cmd := p.GenerateNextCommand()
	if cmd.Code != OpIO {
		t.Fatalf("code = %v, want %v", cmd.Code, OpIO)
	}
	if cmd.Arg1 < 1 || cmd.Arg1 > MaxIOBurst {
		t.Fatalf("IO burst = %d, want 1..%d", cmd.Arg1, MaxIOBurst)
	}
}

func TestGenerateNextCommandCompute(t *testing.T) {
	p := PSW{PC: 0, TotalCommands: 10, IOProbability: 0}

	cmd := p.GenerateNextCommand()
	if cmd.Code != OpCompute {
		t.Fatalf("code = %v, want %v", cmd.Code, OpCompute)
	}
	if cmd.Arg1 < 0 || cmd.Arg1 >= 100 || cmd.Arg2 < 0 || cmd.Arg2 >= 100 {
		t.Fatalf("operands out of range: %d, %d", cmd.Arg1, cmd.Arg2)
	}
}

func TestNewProcessIsReady(t *testing.T) {
	p := New(1, 100, 3)

	if p.State != StateReady {
		t.Fatalf("state = %q, want %q", p.State, StateReady)
	}
	if p.PC != 0 {
		t.Fatalf("pc = %d, want 0", p.PC)
	}
	if p.TotalCommands < MinTotalCommands || p.TotalCommands > MaxTotalCommands {
		t.Fatalf("total commands = %d, want %d..%d", p.TotalCommands, MinTotalCommands, MaxTotalCommands)
	}
	if p.IOProbability < MinIOProbability || p.IOProbability > MaxIOProbability {
		t.Fatalf("io probability = %v, want %v..%v", p.IOProbability, MinIOProbability, MaxIOProbability)
	}
}
