function client(port)
%   provides a menu for accessing PIC32 motor control functions
%
%   client(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.
   
% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',120); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
while ~has_quit
    fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
    % display the menu options; this list will grow
    fprintf('     a: Read Current (counts)    b: Read Current (mA)         c: Read Encoder (counts)    d: Read Encoder (deg)\n');
    %fprintf('     c: Read Encoder (counts)    d: Read Encoder (deg)\n');
    fprintf('     e: Reset Encoder            f: Set PWM (-100 : 100)      g: Set Current Gains        h: Get Current Gains\n');
    %fprintf('     g: Set Current Gains        h: Get Current Gains\n');
    fprintf('     i: Set Position Gains       j: Get Position Gains        k: Run Current Test         l: Go To Angle\n');
    %fprintf('     k: Run Current Test         l: Go To Angle\n');
    fprintf('     m: Set Step Trajectory      n: Set Cubic Trajectory      o: Run Trajectory Tracking  q: Quit\n');
    %fprintf('     o: Run Trajectory Tracking  r: Get Mode\n');
    fprintf('     r: Get Mode\n');
    % read the user's choice
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);
    
    % take the appropriate action
    switch selection
        case 'a' 
            counts = fscanf(mySerial,'%d\n'); 
            fprintf('ADC = %d counts.\n',counts);
  
        case 'b' 
            volts = fscanf(mySerial,'%f\n'); 
            fprintf('ADC = %f mA.\n',volts);
  
        case 'c' 
            counts = fscanf(mySerial,'%d\n'); 
            fprintf('The motor angle is %d counts.\n',counts);
  
        case 'd'                      
            counts = fscanf(mySerial,'%f'); 
            fprintf('The motor angle is %f degrees.\n',counts);
            
         case 'e'
            counts = fscanf(mySerial,'%d'); 
            fprintf('The motor angle is reset to %d counts.\n',counts);
            
         case 'f'
            duty = input('Enter duty cycle (-100 to 100): '); 
            fprintf(mySerial, '%d\n',duty);
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x); 
            
         case 'g'
            Ikp = input('Enter I_Kp gain [4 recommended]: '); 
            fprintf(mySerial, '%f\n',Ikp);
            Iki = input('Enter I_Ki gain [0.3 recommended]: ');
            fprintf(mySerial, '%f\n',Iki);
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x);       
            
        case 'h'
            Ikp = fscanf(mySerial,'%f'); 
            fprintf('Kp: %f\n',Ikp);
            Iki = fscanf(mySerial,'%f'); 
            fprintf('Ki: %f\n',Iki);
            
        case 'i'
            Pkp = input('Enter P_Kp gain [30 recommended]: '); 
            fprintf(mySerial, '%f\n',Pkp);
            Pki = input('Enter P_Ki gain [0 recommended]: ');
            fprintf(mySerial, '%f\n',Pki);
            Pkd = input('Enter P_Kd gain [10 recommended]: ');
            fprintf(mySerial, '%f\n',Pkd);
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x);       
            
        case 'j'
            Pkp = fscanf(mySerial,'%f'); 
            fprintf('P_Kp: %f\n',Pkp);
            Pki = fscanf(mySerial,'%f'); 
            fprintf('P_Ki: %f\n',Pki);
            Pkd = fscanf(mySerial,'%f'); 
            fprintf('P_Kd: %f\n',Pkd);
            
        case 'k'
            read_plot_matrix(mySerial, 'current')
            
        case 'l'
            ang = input('Set angle (deg): '); 
            fprintf(mySerial, '%d\n',ang);
            
        case 'm'
            A = input('Enter Step Trajectory [5 sec max]: ');
            ref = genRef(A, 'step');
            [M,N] = size(ref);
            %fprintf('M: %d\n', M);
            %fprintf('N: %d\n', N);
            %fprintf('ref[500]: %d\n', ref(1));
            
            fprintf(mySerial, '%d\n',N);
            for i= 1:N
                fprintf(mySerial, '%f\n',ref(i));
                %fprintf('i: %d\n', i);
            end
            
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x);   
            
        case 'n'
            A = input('Enter Step Trajectory [5 sec max]: ');
            ref = genRef(A, 'cubic');
            [M,N] = size(ref);
            %fprintf('M: %d\n', M);
            %fprintf('N: %d\n', N);
            %fprintf('ref[500]: %d\n', ref(1));
            
            fprintf(mySerial, '%d\n',N);
            for i= 1:N
                fprintf(mySerial, '%f\n',ref(i));
                %fprintf('i: %f\n', ref(i));
            end
            
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x);   
            
        case 'o'
            read_plot_matrix(mySerial, 'position')
            
        case 'r'
            mode = fscanf(mySerial,'%d'); 
            if (mode == 0)
                fprintf('The mode is IDLE\n');
            elseif (mode == 1)
                fprintf('The mode is PWM\n');

            elseif (mode == 2)
                fprintf('The mode is ITEST\n');
                
            elseif (mode == 3)
                fprintf('The mode is HOLD\n');
                
            elseif (mode == 4)
                fprintf('The mode is TRACK\n');
            end
            
            %fprintf('The mode is %d\n',mode);
            %fprintf('0 = IDLE          1 = PWM\n');
            %fprintf('2 = ITEST         3 = HOLD\n');
            %fprintf('4 = TRACK\n');
            
        case 'p'
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x); 
            
        case 'q'
            has_quit = true;             % exit client
        
        case 'x'
            n = input('Enter number: '); 
            fprintf(mySerial, '%d\n',n);
            m = input('Enter number: ');
            fprintf(mySerial, '%d\n',m);
            x = fscanf(mySerial,'%d');   
            fprintf('Read: %d\n',x);       
            
        otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end
