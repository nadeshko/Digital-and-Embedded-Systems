% SineWaveTableGenerator
% Generate a table for Integrated Mechatronic Project

clear
% Always a good idea to place at the start of every script

NumberOfPoints = 32;
NumberOfBits = 12;
Offset = 2047.5;% This is a surprisingly challenging bit to add
ScaleFactor = 2047.5;% With 2^N levels, both the scale and offset will be y+0.5
%Need to add in extra control bits -see Page 22 of MCP4801 data sheet
GA = 0 * 2^13;     
%Gain control bit -set to zero
SHDN = 1 * 2^12;    
%Shut-down bit -set to one
WaveTableIndex = 0:(NumberOfPoints-1);
WaveTable = Offset + ScaleFactor*sin(2*2*pi*WaveTableIndex/NumberOfPoints);
% Check we got scaling correct
MinAmplitude = min(WaveTable);
MaxAmptitude = max(WaveTable);

% At this point you can add some sophisticated error checking to make sure you
% have got things correct
plot(WaveTable)

% It might be nice to display the data graphically at this point?
% Include the control bits -this is a 
% logical OR (binary addition provided the existing bits and always zero)
WaveTable = WaveTable + GA + SHDN;
% Convert to a hex number in a format that we can paste directly into the editor
% In C, format will be "const int16 WaveTable[32]={0,1,2...31}"

fprintf('const int16 SineWave_TwoKHZ[32]={')
for WaveTableIndex = 1:NumberOfPoints% Convert to a 4-digit hex number
    fprintf('0x%s',dec2hex(round(WaveTable(WaveTableIndex)),4)) 
    if WaveTableIndex < NumberOfPoints
        fprintf(',')
    end
end
fprintf('};')
% Now you need to add code to generate the 2kHz and the 1kHz+2kHz tables
% Error checking becomes even more importantwith the third table