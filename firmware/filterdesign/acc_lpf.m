% Accelerometer low pass filter
% Everything above f_cutoff is considered to be noise
sampling_period = 0.002;
f_cutoff = 18.0;
omega_cutoff = 2 * pi * f_cutoff;
filterOrder = 20;
t = -(filterOrder - 1) * sampling_period / 2:sampling_period:(filterOrder - 1) * sampling_period / 2;

filter = pi / omega_cutoff * sinc(omega_cutoff * t / pi);
figure(1);
plot(filter);
%figure(2);
%plot(diff(conv(filter, signal)));
disp("Filter output:");
sprintf("%0.5f, ", filter(:))