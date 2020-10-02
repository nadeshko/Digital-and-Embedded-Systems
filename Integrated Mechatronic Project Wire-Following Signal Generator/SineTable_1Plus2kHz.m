% SineWaveTableGenerator
clear % start with clearing workspace

NumberOfPoints = 32;     % 32kHz with 32 samples
NumberOfBits = 12;       % amount of bits MCP will receive
Offset = 2047.5;         % Max voltage is 4.095V
ScaleFactor = 2047.5;    % Amplitude size from center of offset

%Need to add in extra control bits, GA and SHDN
%Shut-down bit set to one, 12th bit from the right
SHDN = 1 * 2^12;  
%Gain control bit set to zero, 13th bit from right
GA = 0 * 2^13; 

WaveTableIndex = 0:(NumberOfPoints-1);
Sine1 = Offset + ScaleFactor*sin(2*pi*WaveTableIndex/NumberOfPoints);   %1kHz sinewave
Sine2 = Offset + ScaleFactor*sin(2*2*pi*WaveTableIndex/NumberOfPoints); %2kHz sinewave
FinalSine = (Sine1+Sine2)*0.5  %1+2kHz, multiplied by 0.5 to keep it below 4.095V

% Checking to see if max and min amplitude is over 4095 or under 0
MinAmplitude = min(FinalSine); %FinalSine can be replaced with Sine1 or Sine2
MaxAmptitude = max(FinalSine);

%producing graphs to check again and compare waveforms
subplot(3,1,1)
plot(Sine1)
subplot(3,1,2)
plot(Sine2)
subplot(3,1,3)
plot(FinalSine)

% Include the control bits into hexadecimal code since its a 16 bit data
FinalSine = FinalSine + GA + SHDN;

% Convert to a hex number in a format that we can paste directly into the editor
fprintf('const int16 Combined_Wave[32]={')
for WaveTableIndex = 1:NumberOfPoints % Convert to a 4-digit hex number
    fprintf('0x%s',dec2hex(round(FinalSine(WaveTableIndex)),4)) 
    if WaveTableIndex < NumberOfPoints
        fprintf(',')
    end
end
fprintf('};')