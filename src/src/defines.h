#ifndef DEFINES_H
#define DEFINES_H

#define MOCKING "mocking.csv"       // Uncomment to enable mocking mode, reading data from SD card rather than from sensors (format: time (ms), bus voltage (V), shunt voltage (mV))
#define MOCKING_3221 "mock_3221.csv"// Path for INA3221 mocking file (format: voltage across cell 1, 2, 3, 4 (all in V))
#define CURRENT_FILE "curr.txt"     // File path to read/ store average current over several flights
#define WATT_FILE "watt.txt"        // File path to read/ store average wattage over several flights
#define SERIAL_TIMEOUT 0            // Time to wait for serial connection (s)
#define USE_DISPLAY 1               // Display toggle
#define SD_LOGGING 1                // SD Logging toggle
#define MAX_RESTING_AMPS 1          // Maximum current for initial battery stabilisation
#define MIN_FLYING_AMPS 1           // Minimum current for flying state, should be set to current when motors are idling
#define CAPACITY_STEP_PERCENTAGE 1  // Percentage of capacity to decrease when iterating simulation capacity
#define MAX_VOLTAGE_VARIANCE 0.2    // Maximum difference in voltage between simulation and real performance before adjusting simulation
#define MAX_CELL_VARIANCE 0.1       // Maximum difference between cells before warning
#define SHORT_DECAY_SECONDS 180     // Decay time for per-flight exponential moving average
#define LONG_DECAY_SECONDS 1800     // Decay time for long-term exponential moving average
#define MIN_FLYING_VOLTAGE 13.5     // Minimum charge percentage in decimal

#define INTERNAL_RESISTANCE 0.0208  // Internal resistance of battery (Ohms)
#define CAPACITY 1.55               // Battery capacity (Amp hours)
#define NOMINAL_VOLTAGE 14.75       // Voltage at end of "nominal zone", just before final exponential voltage drop
#define NOMINAL_CAPACITY 1.42       // Capacity used at end of "nominal zone" (Amp hours)
#define EXPONENTIAL_VOLTAGE 15.26   // Voltage at end of initial exponential zone
#define EXPONENTIAL_CAPACITY 0.8    // Capacity at end of initial exponential zone
#define CURVE_CURRENT 0.06340690796660907   // Discharge current during test used to extract parameters
#define MAX_VOLTAGE 16.8            // Max and min voltages used for SoC estimation and comparison, ensure that the
#define MIN_VOLTAGE 13.3            // number of millivolts between these is equal to the length of the lookup array.
#define REACTION_TIME 30            // Battery reaction time
const float MAH_AT_VOLTAGE[] = {1541.9099999999999, 1541.9099999999999, 1541.9099999999999, 1541.9099999999999, 1541.9099999999999, 1541.9099999999999, 1541.889999997616, 1541.8071432059157, 1541.2999999999997, 1541.2599999999998, 1540.6573333333336, 1540.35, 1540.2946456500677, 1539.4193307291619, 1539.255, 1539.0073326771653, 1538.2999999999997, 1538.2599999999998, 1537.7546458231604, 1537.2999999999952, 1537.2593748424852, 1536.6649193948413, 1536.0242857156481, 1535.5199999999954, 1535.3546456693007, 1534.4799999999998, 1534.4299999999998, 1533.7885728250244, 1533.4099999999999, 1533.0985832923225, 1532.4499999999907, 1532.3999999999894, 1531.5373385521746, 1531.429999998808, 1530.9796774195088, 1530.34866145577, 1529.7399999999998, 1529.5974603223044, 1529.1385728346452, 1528.1099999999997, 1528.0499999999997, 1527.3893750000002, 1526.7874206349206, 1525.9773333333337, 1525.7399999999998, 1524.8273229163487, 1524.6399999999999, 1524.1, 1523.15, 1523.0899998365985, 1522.0699999999997, 1521.8643331692913, 1521.07998046875, 1520.6285826771607, 1519.9999999999254, 1519.2773333333334, 1518.98, 1518.0899999999997, 1517.8596771043344, 1517.0043307088117, 1516.3899999999999, 1515.7293548380944, 1515.5275000000001, 1514.46, 1514.0473216043174, 1513.3399999999997, 1512.4185836589336, 1512.237322834645, 1511.3399804687501, 1510.5885716029788, 1510.1599999999999, 1509.3448387096773, 1508.3446673177878, 1507.8899999999999, 1506.9099999999999, 1506.1746458333332, 1505.8271456476623, 1504.7699999999627, 1503.8899999999999, 1503.4771456692902, 1502.77, 1501.7999999999997, 1501.0193333384202, 1500.6485728345706, 1499.76998046875, 1498.8199999999997, 1498.1271456476625, 1497.584285715648, 1496.54, 1495.4499999999998, 1494.7973229166664, 1494.227142857143, 1493.2999999999997, 1492.1999999999998, 1491.2585826819736, 1490.6886666667065, 1489.5499999999997, 1488.5899999999997, 1487.4799999999977, 1486.3986669921876, 1485.77935416921, 1484.6546456693009, 1483.818661417325, 1482.88, 1481.7599999999998, 1480.79, 1479.6799999999998, 1478.5699999237058, 1477.739999999851, 1476.7599999999998, 1475.6399951171875, 1474.5099999999998, 1473.3899999999999, 1472.3073333740235, 1471.218666666667, 1470.1273332926532, 1469.2187096780347, 1467.967459697118, 1466.4573216043314, 1465.0749999999998, 1463.94, 1462.9499999999998, 1461.8199999999997, 1460.5492913386015, 1459.167143205958, 1457.6846666668257, 1456.4499999618529, 1455.4399999904633, 1453.868709697108, 1452.0693333333334, 1450.64998046875, 1449.6685714285722, 1447.7185833333333, 1446.42, 1444.5046456500677, 1442.8446456704928, 1441.2599999809263, 1439.0799206446086, 1436.6393334150316, 1434.46, 1431.7346653639474, 1428.5142912946535, 1424.8693749999998, 1419.3474166666665, 1412.1486614557903, 1402.1986666475932, 1387.2692913385827, 1366.4142858014895, 1338.2342864125121, 1324.667333330789, 1317.6493548380945, 1311.6585832923229, 1305.8486614173228, 1300.1386666667067, 1294.0146458326976, 1286.5685833333332, 1278.5786666666615, 1269.3693651568701, 1259.3771456473642, 1248.5892913409857, 1238.1285728236821, 1228.6585714312992, 1220.0346666666771, 1211.0985714285714, 1201.1993333333348, 1191.2449194335939, 1180.9571665846456, 1169.637333334605, 1156.55858333077, 1142.7874193548387, 1128.9385714503724, 1114.9846456500463, 1102.2143307086426, 1091.1485826002702, 1081.6546666615804, 1072.0346458326976, 1061.8792864125123, 1051.7692916666665, 1042.2642857142857, 1031.814291300093, 1021.2986608014894, 1011.3871653543306, 1001.7686607251956, 990.9886693523777, 980.537322834645, 969.5593346774181, 957.3942858014893, 945.1385825906482, 931.3146653645833, 916.4187083384202, 900.4574206155349, 889.0286666666691, 876.4846665852865, 862.7571653543307, 850.0293307298025, 838.7742745535713, 826.1685728250338, 814.0071432062562, 802.7371428557805, 792.3971432059577, 781.2898412698412, 770.4042857142856, 760.1273333333334, 750.304285714307, 740.604642857824, 729.7071456692902, 720.4485825892857, 711.494285714307, 702.3192913001351, 693.4685714289121, 684.017165178912, 675.4293307291666, 667.3746667073572, 658.8771653543306, 650.6574168395996, 642.6443307086627, 634.7843307086625, 627.3573216043308, 619.74484375, 612.1643304010827, 605.8896774193549, 598.5393346355759, 591.7173326822905, 585.0573332926432, 578.6173228250339, 572.4087096381188, 565.9673216043213, 559.9043307086613, 554.0887096774386, 548.2192864172853, 542.3186666666668, 536.8085728346456, 531.1786614125115, 525.9243307086613, 520.6093333333347, 515.084643206256, 509.7686666653951, 504.6973214503723, 499.43919642857145, 494.4471428571428, 489.4993333333333, 484.53714566929136, 479.78464320864265, 475.05929166666635, 470.35, 465.60714320595775, 460.97929130013534, 456.19714564766207, 451.4373229115406, 446.8385714285714, 441.8342858014894, 436.67464320881163, 431.76466666666664, 426.2693331705729, 420.9086608015647, 414.75732160312805, 408.8873437499994, 402.88929166658716, 396.9373216031281, 391.074833659511, 385.0346667073571, 379.2473333333333, 374.2546771037182, 368.78716666664684, 363.82857143129615, 359.06733333079023, 354.4098333307902, 349.977145649711, 345.67732283524674, 341.62, 337.53866682944704, 333.8443307086626, 330.59499999999997, 327.0342912953241, 323.7985826819713, 320.6693542098999, 317.854642857228, 314.85, 312.11935483870997, 309.0899999999999, 306.5693334147136, 304.207142857824, 301.38483333460505, 298.9799999999989, 296.4592864119162, 294.15, 291.6671665852467, 289.23997983871, 286.44935483901736, 283.784285714456, 281.02937499999996, 278.54714285714283, 275.4799987792969, 272.40929133858265, 269.13732160433074, 265.65999999999997, 261.67464285722804, 257.128582589456, 252.10742061553492, 246.47484126923575, 239.7546458230807, 232.51866666666172, 224.15929164616261, 215.5096825787378, 208.76935483932493, 202.9142857142857, 197.51732291158027, 192.69999991280687, 187.94464565006766, 183.27999999999997, 178.6667857142857, 173.91999999983113, 169.32936507936512, 164.55936507936508, 159.89467740950985, 155.59464567049338, 151.12937254901968, 146.8446771043154, 142.49733335368086, 138.1074169921875, 133.69433040108265, 129.1386663411359, 124.46484123108849, 120.17873015388847, 115.8599206350732, 111.70491943359374, 107.50933072789448, 102.86968257874014, 98.32742061493857, 93.91733365889398, 89.47983878968225, 85.09484126923576, 80.95866682944694, 76.63871030776747, 72.469921875, 68.2671665852464, 63.73984129898221, 59.1946771037194, 54.69464566929141, 50.3573326822718, 45.94999999998078, 41.638710289001466, 37.51492156505584, 33.409841269846, 29.09870967741997, 24.978582590648273, 20.479999999999713, 16.30428571428572, 12.2386614583234, 8.319354838719292};
const float MWH_AT_VOLTAGE[] = {23790.554841229838, 23790.554841229838, 23790.554841229838, 23790.554841229838, 23790.554841229838, 23790.554841229838, 23790.139364919356, 23788.969687194527, 23782.28483870968, 23781.639364919356, 23773.526666782927, 23769.41483871214, 23768.53125977518, 23756.905833308352, 23754.59483870968, 23751.242677165355, 23741.868709677416, 23741.169677419508, 23734.391417399744, 23728.22935515373, 23727.538697596232, 23719.628549107147, 23711.01929134246, 23704.209838709678, 23701.88003913894, 23690.00967741936, 23689.258709677437, 23680.59996089935, 23675.47871030746, 23671.1053543307, 23662.314838788432, 23661.559355153593, 23649.84125007636, 23648.25967741936, 23642.09333170553, 23633.452661481242, 23625.234838788434, 23623.127460629905, 23616.892501221897, 23602.93741935484, 23602.019677419354, 23592.99452620968, 23584.67428571402, 23573.646785715653, 23570.319354848554, 23557.851417338054, 23555.17935483872, 23547.78983870968, 23534.834838709758, 23533.809675922406, 23520.02741935484, 23517.0350024426, 23506.157419394214, 23499.920312805094, 23491.18741935484, 23481.163333333338, 23476.97742061492, 23464.698709677417, 23461.51066682943, 23449.60125061095, 23441.10741935484, 23431.870625157695, 23428.89871030746, 23414.13745967742, 23408.28000977353, 23398.459677419356, 23385.58666667073, 23382.950156402665, 23370.447419354838, 23359.927165200395, 23353.89742061492, 23342.470317540487, 23328.59428571431, 23322.14871030746, 23308.43741935484, 23298.015354353585, 23293.009990213955, 23278.16483871214, 23265.72935515373, 23259.789980463982, 23249.804841229838, 23236.17935483872, 23225.003571602996, 23219.65499511241, 23207.18741935484, 23193.877419354838, 23183.998572825025, 23176.24571419852, 23161.498709678035, 23146.06870969711, 23136.701417322838, 23128.475708705286, 23115.369677419352, 23099.704838709677, 23086.230001221935, 23078.017142857825, 23061.99741935607, 23048.17967741936, 23032.314838788432, 23016.87583732359, 23007.978572825028, 22991.91984375, 22979.91999023199, 22966.448709677417, 22950.378709677418, 22936.409354848554, 22920.459677419356, 22904.444838709678, 22892.47935515373, 22878.30935515373, 22862.14967757687, 22845.93870967742, 22829.72742061492, 22814.04666721221, 22798.31285807287, 22782.492505537666, 22769.300001383468, 22751.152916463194, 22729.414354246852, 22709.35483871214, 22692.82967741951, 22678.377419354838, 22661.954838709677, 22643.439990228413, 22623.326666666766, 22601.78141886393, 22583.67935483872, 22568.87935483871, 22546.210669270913, 22520.20133858268, 22499.307420614918, 22484.890634940006, 22456.41002083847, 22437.31870969711, 22409.334291338622, 22384.927497556208, 22361.52741935484, 22329.587167619986, 22293.736666687706, 22261.53983870968, 22221.340004885198, 22173.791339284355, 22119.938044394217, 22038.102834799927, 21931.718749923704, 21784.37535433071, 21563.459373778103, 21254.230714308418, 20836.303333333348, 20635.28267821312, 20530.89937255383, 20441.699990215282, 20355.669988851507, 20270.86535441081, 20179.960156402663, 20069.07000490324, 19949.88566928752, 19812.698568638392, 19663.865357137423, 19502.738572825034, 19346.591417410546, 19204.752678569057, 19075.590708796204, 18941.930156556373, 18793.601417338054, 18644.47665763613, 18489.90992172226, 18320.182834645668, 18124.039922483767, 17917.074998245203, 17708.425669642176, 17497.976770833287, 17305.927322825024, 17139.214166666676, 16996.010078163185, 16851.019843597373, 16697.68666666667, 16544.837500004855, 16401.040078430775, 16243.026666666667, 16083.962913307148, 15934.214285801565, 15789.626612904456, 15627.228708419201, 15469.327142857142, 15303.084833669363, 15118.705322963084, 14933.865713674, 14726.000000153224, 14500.711614584154, 14258.34125248083, 14084.650078776267, 13894.030078201331, 13685.80333417339, 13492.18270084892, 13321.384920615841, 13130.43322895686, 12945.350356800944, 12773.614645956717, 12615.870333426392, 12446.241666666667, 12279.729921865463, 12122.74015877016, 11972.260000038223, 11823.820009844461, 11656.666666667077, 11514.801259841726, 11377.409998779296, 11236.623228352866, 11100.550002451093, 10955.200078755064, 10823.399921875152, 10699.912583667388, 10569.864193550859, 10443.099818257979, 10319.80838725438, 10199.2085483878, 10084.656666673025, 9967.54691283701, 9850.412903544107, 9753.40984375, 9639.882668813072, 9534.694921798706, 9431.407500000001, 9331.632083333334, 9235.69858367174, 9135.672903620401, 9041.506692708335, 8951.479372557402, 8860.24580217634, 8768.310009770546, 8682.826458333333, 8595.195826612864, 8513.033333543346, 8430.270019847241, 8344.251428568703, 8261.34, 8181.425166341107, 8099.159821078437, 8020.867416834756, 7943.300039100684, 7865.390714465802, 7790.791607863059, 7716.617420654285, 7642.567480161171, 7567.901254892367, 7495.165725885829, 7419.840634920636, 7344.963346459483, 7272.487333354875, 7193.480078278799, 7112.03135091151, 7034.530000305176, 6947.707333674412, 6862.921333699918, 6765.535333333652, 6672.5767382043505, 6577.637338714451, 6483.242661458334, 6390.225833336636, 6294.340317545575, 6202.44499023199, 6123.0616658512745, 6036.06984252451, 5957.197497558444, 5881.349999847413, 5807.125895917339, 5736.440647125584, 5667.8683335440555, 5603.16748031374, 5537.950004964645, 5478.885335184759, 5426.938740157475, 5369.960002441411, 5318.185807241605, 5268.044969463046, 5222.918749389639, 5174.7196850201435, 5130.904666709601, 5082.2987103175365, 5041.760002443829, 5003.738588541706, 4958.350317546216, 4919.60748016117, 4879.080078435098, 4841.907479848562, 4801.7767842756, 4762.597343138452, 4717.589997558594, 4674.514666707477, 4630.0237401574805, 4589.787142857143, 4540.109365080576, 4490.471333792905, 4437.395333415616, 4381.037421254961, 4316.284841270149, 4242.389960935116, 4160.81000000015, 4069.3025024607873, 3959.8964516129035, 3842.019286784521, 3705.766653238137, 3564.6466682942832, 3454.6387497037554, 3358.987086703394, 3270.823333336538, 3192.0974180821013, 3114.2853333391463, 3038.0198412310883, 2962.4514313728578, 2884.718730156932, 2809.490317540328, 2731.225333333356, 2654.653332926432, 2584.043229115804, 2510.6699996948237, 2440.205825983483, 2368.660002460797, 2296.383308095782, 2223.6550198462705, 2148.6135416921475, 2071.4800048828147, 2000.7503125190688, 1929.4929032389323, 1860.770313033452, 1791.453333231608, 1714.653229173075, 1639.4249902152828, 1566.380009844383, 1492.8900781637456, 1420.1125000381844, 1351.490157475509, 1279.73, 1210.546572108193, 1140.603225796617, 1065.3271771037194, 989.642678572337, 914.7151614584461, 842.4335420768494, 768.959841288915, 696.9974799250055, 628.15354909047, 559.6506666673081, 487.51871031165126, 418.6698425245098, 343.3549193548399, 273.423336590144, 205.35333334615183, 139.6386614584154};

// Values for INA226 onboard shunt
// #define CURRENT_OFFSET -1.63992595  // mA
// #define CURRENT_SCALE 999.5734377   // mV/10A
// Values for 200A shunt:
#define CURRENT_OFFSET -982.8158579
#define CURRENT_SCALE 3.7834625

#define A_ms_to_A_h 0.00000027777777777778
#define shuntVoltageTomA(mV) (((mV) / CURRENT_SCALE) * 10000 + CURRENT_OFFSET)
#define STABILISATION_TIME_MS 5000      // Time to allow voltage to settle after initial powerup
#define MAX_ZERO_CURRENT_TIME_MS 1000   // Max zero current time before changing from flying state to not flying

#endif