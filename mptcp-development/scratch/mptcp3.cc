/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Morteza Kheirkhah <m.kheirkhah@sussex.ac.uk>
 */

// Network topology
//
//       n0 ----------- n1
// - Flow from n0 to n1 using MpTcpBulkSendApplication.

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
// #include <unistd.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MpTcpBulkSendExample");
bool enable_qcn = true, use_dynamic_pfc_threshold = true, packet_level_ecmp = true, flow_level_ecmp = false;
uint32_t packet_payload_size = 1000, l2_chunk_size = 0, l2_ack_interval = 0;
double pause_time = 5, simulator_stop_time = 3.01, app_start_time = 1.0, app_stop_time = 9.0;
std::string data_rate, link_delay, topology_file, flow_file, tcp_flow_file, trace_file, trace_output_file;
bool used_port[65536] = { 0 };

double cnp_interval = 50, alpha_resume_interval = 55, rp_timer, dctcp_gain = 1 / 16, np_sampling_interval = 0, pmax = 1;
uint32_t byte_counter, fast_recovery_times = 5, kmax = 60, kmin = 60;
std::string rate_ai, rate_hai;

bool clamp_target_rate = false, clamp_target_rate_after_timer = false, send_in_chunks = false, l2_wait_for_ack = false, l2_back_to_zero = false, l2_test_read = false;
double error_rate_per_link = 0.0;
int
main(int argc, char *argv[])
{

  clock_t begint, endt;
  begint = clock();

  
#ifndef PGO_TRAINING
  if (argc > 1)
#else
  if (true)
#endif
  {
    //Read the configuration file
    std::ifstream conf;
#ifndef PGO_TRAINING
    conf.open(argv[1]);
#else
    conf.open(PATH_TO_PGO_CONFIG);
#endif
    // int access(argv[1], int mode);
    while (!conf.eof())
    {
      std::string key;
      conf >> key;


      //std::cout << conf.cur << "\n";

      if (key.compare("ENABLE_QCN") == 0)
      {
        uint32_t v;
        conf >> v;
        enable_qcn = v;
        if (enable_qcn)
          std::cout << "ENABLE_QCN\t\t\t" << "Yes" << "\n";
        else
          std::cout << "ENABLE_QCN\t\t\t" << "No" << "\n";
      }
      else if (key.compare("USE_DYNAMIC_PFC_THRESHOLD") == 0)
      {
        uint32_t v;
        conf >> v;
        use_dynamic_pfc_threshold = v;
        if (use_dynamic_pfc_threshold)
          std::cout << "USE_DYNAMIC_PFC_THRESHOLD\t" << "Yes" << "\n";
        else
          std::cout << "USE_DYNAMIC_PFC_THRESHOLD\t" << "No" << "\n";
      }
      else if (key.compare("CLAMP_TARGET_RATE") == 0)
      {
        uint32_t v;
        conf >> v;
        clamp_target_rate = v;
        if (clamp_target_rate)
          std::cout << "CLAMP_TARGET_RATE\t\t" << "Yes" << "\n";
        else
          std::cout << "CLAMP_TARGET_RATE\t\t" << "No" << "\n";
      }
      else if (key.compare("CLAMP_TARGET_RATE_AFTER_TIMER") == 0)
      {
        uint32_t v;
        conf >> v;
        clamp_target_rate_after_timer = v;
        if (clamp_target_rate_after_timer)
          std::cout << "CLAMP_TARGET_RATE_AFTER_TIMER\t" << "Yes" << "\n";
        else
          std::cout << "CLAMP_TARGET_RATE_AFTER_TIMER\t" << "No" << "\n";
      }
      else if (key.compare("PACKET_LEVEL_ECMP") == 0)
      {
        uint32_t v;
        conf >> v;
        packet_level_ecmp = v;
        if (packet_level_ecmp)
          std::cout << "PACKET_LEVEL_ECMP\t\t" << "Yes" << "\n";
        else
          std::cout << "PACKET_LEVEL_ECMP\t\t" << "No" << "\n";
      }
      else if (key.compare("FLOW_LEVEL_ECMP") == 0)
      {
        uint32_t v;
        conf >> v;
        flow_level_ecmp = v;
        if (flow_level_ecmp)
          std::cout << "FLOW_LEVEL_ECMP\t\t\t" << "Yes" << "\n";
        else
          std::cout << "FLOW_LEVEL_ECMP\t\t\t" << "No" << "\n";
      }
      else if (key.compare("PAUSE_TIME") == 0)
      {
        double v;
        conf >> v;
        pause_time = v;
        std::cout << "PAUSE_TIME\t\t\t" << pause_time << "\n";
      }
      else if (key.compare("DATA_RATE") == 0)
      {
        std::string v;
        conf >> v;
        data_rate = v;
        std::cout << "DATA_RATE\t\t\t" << data_rate << "\n";
      }
      else if (key.compare("LINK_DELAY") == 0)
      {
        std::string v;
        conf >> v;
        link_delay = v;
        std::cout << "LINK_DELAY\t\t\t" << link_delay << "\n";
      }
      else if (key.compare("PACKET_PAYLOAD_SIZE") == 0)
      {
        uint32_t v;
        conf >> v;
        packet_payload_size = v;
        std::cout << "PACKET_PAYLOAD_SIZE\t\t" << packet_payload_size << "\n";
      }
      else if (key.compare("L2_CHUNK_SIZE") == 0)
      {
        uint32_t v;
        conf >> v;
        l2_chunk_size = v;
        std::cout << "L2_CHUNK_SIZE\t\t\t" << l2_chunk_size << "\n";
      }
      else if (key.compare("L2_ACK_INTERVAL") == 0)
      {
        uint32_t v;
        conf >> v;
        l2_ack_interval = v;
        std::cout << "L2_ACK_INTERVAL\t\t\t" << l2_ack_interval << "\n";
      }
      else if (key.compare("L2_WAIT_FOR_ACK") == 0)
      {
        uint32_t v;
        conf >> v;
        l2_wait_for_ack = v;
        if (l2_wait_for_ack)
          std::cout << "L2_WAIT_FOR_ACK\t\t\t" << "Yes" << "\n";
        else
          std::cout << "L2_WAIT_FOR_ACK\t\t\t" << "No" << "\n";
      }
      else if (key.compare("L2_BACK_TO_ZERO") == 0)
      {
        uint32_t v;
        conf >> v;
        l2_back_to_zero = v;
        if (l2_back_to_zero)
          std::cout << "L2_BACK_TO_ZERO\t\t\t" << "Yes" << "\n";
        else
          std::cout << "L2_BACK_TO_ZERO\t\t\t" << "No" << "\n";
      }
      else if (key.compare("L2_TEST_READ") == 0)
      {
        uint32_t v;
        conf >> v;
        l2_test_read = v;
        if (l2_test_read)
          std::cout << "L2_TEST_READ\t\t\t" << "Yes" << "\n";
        else
          std::cout << "L2_TEST_READ\t\t\t" << "No" << "\n";
      }
      else if (key.compare("TOPOLOGY_FILE") == 0)
      {
        std::string v;
        conf >> v;
        topology_file = v;
        std::cout << "TOPOLOGY_FILE\t\t\t" << topology_file << "\n";
      }
      else if (key.compare("FLOW_FILE") == 0)
      {
        std::string v;
        conf >> v;
        flow_file = v;
        std::cout << "FLOW_FILE\t\t\t" << flow_file << "\n";
      }
      else if (key.compare("TCP_FLOW_FILE") == 0)
      {
        std::string v;
        conf >> v;
        tcp_flow_file = v;
        std::cout << "TCP_FLOW_FILE\t\t\t" << tcp_flow_file << "\n";
      }
      else if (key.compare("TRACE_FILE") == 0)
      {
        std::string v;
        conf >> v;
        trace_file = v;
        std::cout << "TRACE_FILE\t\t\t" << trace_file << "\n";
      }
      else if (key.compare("TRACE_OUTPUT_FILE") == 0)
      {
        std::string v;
        conf >> v;
        trace_output_file = v;
        if (argc > 2)
        {
          trace_output_file = trace_output_file + std::string(argv[2]);
        }
        std::cout << "TRACE_OUTPUT_FILE\t\t" << trace_output_file << "\n";
      }
      else if (key.compare("APP_START_TIME") == 0)
      {
        double v;
        conf >> v;
        app_start_time = v;
        std::cout << "SINK_START_TIME\t\t\t" << app_start_time << "\n";
      }
      else if (key.compare("APP_STOP_TIME") == 0)
      {
        double v;
        conf >> v;
        app_stop_time = v;
        std::cout << "SINK_STOP_TIME\t\t\t" << app_stop_time << "\n";
      }
      else if (key.compare("SIMULATOR_STOP_TIME") == 0)
      {
        double v;
        conf >> v;
        simulator_stop_time = v;
        std::cout << "SIMULATOR_STOP_TIME\t\t" << simulator_stop_time << "\n";
      }
      else if (key.compare("CNP_INTERVAL") == 0)
      {
        double v;
        conf >> v;
        cnp_interval = v;
        std::cout << "CNP_INTERVAL\t\t\t" << cnp_interval << "\n";
      }
      else if (key.compare("ALPHA_RESUME_INTERVAL") == 0)
      {
        double v;
        conf >> v;
        alpha_resume_interval = v;
        std::cout << "ALPHA_RESUME_INTERVAL\t\t" << alpha_resume_interval << "\n";
      }
      else if (key.compare("RP_TIMER") == 0)
      {
        double v;
        conf >> v;
        rp_timer = v;
        std::cout << "RP_TIMER\t\t\t" << rp_timer << "\n";
      }
      else if (key.compare("BYTE_COUNTER") == 0)
      {
        uint32_t v;
        conf >> v;
        byte_counter = v;
        std::cout << "BYTE_COUNTER\t\t\t" << byte_counter << "\n";
      }
      else if (key.compare("KMAX") == 0)
      {
        uint32_t v;
        conf >> v;
        kmax = v;
        std::cout << "KMAX\t\t\t\t" << kmax << "\n";
      }
      else if (key.compare("KMIN") == 0)
      {
        uint32_t v;
        conf >> v;
        kmin = v;
        std::cout << "KMIN\t\t\t\t" << kmin << "\n";
      }
      else if (key.compare("PMAX") == 0)
      {
        double v;
        conf >> v;
        pmax = v;
        std::cout << "PMAX\t\t\t\t" << pmax << "\n";
      }
      else if (key.compare("DCTCP_GAIN") == 0)
      {
        double v;
        conf >> v;
        dctcp_gain = v;
        std::cout << "DCTCP_GAIN\t\t\t" << dctcp_gain << "\n";
      }
      else if (key.compare("FAST_RECOVERY_TIMES") == 0)
      {
        uint32_t v;
        conf >> v;
        fast_recovery_times = v;
        std::cout << "FAST_RECOVERY_TIMES\t\t" << fast_recovery_times << "\n";
      }
      else if (key.compare("RATE_AI") == 0)
      {
        std::string v;
        conf >> v;
        rate_ai = v;
        std::cout << "RATE_AI\t\t\t\t" << rate_ai << "\n";
      }
      else if (key.compare("RATE_HAI") == 0)
      {
        std::string v;
        conf >> v;
        rate_hai = v;
        std::cout << "RATE_HAI\t\t\t" << rate_hai << "\n";
      }
      else if (key.compare("NP_SAMPLING_INTERVAL") == 0)
      {
        double v;
        conf >> v;
        np_sampling_interval = v;
        std::cout << "NP_SAMPLING_INTERVAL\t\t" << np_sampling_interval << "\n";
      }
      else if (key.compare("SEND_IN_CHUNKS") == 0)
      {
        uint32_t v;
        conf >> v;
        send_in_chunks = v;
        if (send_in_chunks)
        {
          std::cout << "SEND_IN_CHUNKS\t\t\t" << "Yes" << "\n";
          std::cout << "WARNING: deprecated and not tested. Please consider using L2_WAIT_FOR_ACK";
        }
        else
          std::cout << "SEND_IN_CHUNKS\t\t\t" << "No" << "\n";
      }
      else if (key.compare("ERROR_RATE_PER_LINK") == 0)
      {
        double v;
        conf >> v;
        error_rate_per_link = v;
        std::cout << "ERROR_RATE_PER_LINK\t\t" << error_rate_per_link << "\n";
      }
      fflush(stdout);
    }
    conf.close();
  }
  else
  {
    std::cout << "Error: require a config file\n";
    fflush(stdout);
    return 1;
  }

  // LogComponentEnable("MpTcpSocketBase", LOG_INFO);

  NS_ASSERT(packet_level_ecmp + flow_level_ecmp < 2); //packet level ecmp and flow level ecmp are exclusive
  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
  // Config::SetDefault("ns3::MpTcpSocketBase::OutputFileName", StringValue("mptcptrace.txt"));
  Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("Fully_Coupled"));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagement", StringValue("NdiffPorts"));
  


  SeedManager::SetSeed(time(NULL));

  std::ifstream topof, flowf, tracef, tcpflowf;
  topof.open(topology_file.c_str());
  flowf.open(flow_file.c_str());
  tracef.open(trace_file.c_str());
  tcpflowf.open(tcp_flow_file.c_str());
  uint32_t node_num, switch_num, link_num, flow_num, trace_num, tcp_flow_num;
  topof >> node_num >> switch_num >> link_num;
  flowf >> flow_num;
  tracef >> trace_num;
  tcpflowf >> tcp_flow_num;


  NodeContainer nodes;
  nodes.Create(node_num);
  NS_LOG_INFO("Create nodes.");
  InternetStackHelper internet;
  //internet.Install(n);
  internet.Install(nodes.Get(0));
  internet.Install(nodes.Get(1));
  internet.Install(nodes.Get(4));
  internet.Install(nodes.Get(5));

  InternetStackHelper internet1;
  //Ipv4ListRoutingHelper staticonly;
  //Ipv4ListRoutingHelper staticRouting;
  //staticonly.Add(staticonly, 1);
  //internet1.SetRoutingHelper(staticonly);
  internet1.Install(nodes.Get(2));
  internet1.Install(nodes.Get(3));

  NS_LOG_INFO("Create channels.");
  Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  rem->SetRandomVariable(uv);
  uv->SetStream(50);
  rem->SetAttribute("ErrorRate", DoubleValue(error_rate_per_link));
  rem->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  PointToPointHelper p2plink;
  Ipv4AddressHelper ipv4;
  for (uint32_t i = 0; i < link_num; i++)
  {
    uint32_t src, dst;
    std::string data_rate, link_delay;
    double error_rate;
    topof >> src >> dst >> data_rate >> link_delay >> error_rate;
    p2plink.SetDeviceAttribute("DataRate", StringValue(data_rate));
    p2plink.SetChannelAttribute("Delay", StringValue(link_delay));

    if (error_rate > 0)
    {
      Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
      Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
      rem->SetRandomVariable(uv);
      uv->SetStream(50);
      rem->SetAttribute("ErrorRate", DoubleValue(error_rate));
      rem->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
      p2plink.SetDeviceAttribute("ReceiveErrorModel", PointerValue(rem));
    }
    else
    {
      p2plink.SetDeviceAttribute("ReceiveErrorModel", PointerValue(rem));
    }

    fflush(stdout);
    //devices
    NetDeviceContainer d = p2plink.Install(nodes.Get(src), nodes.Get(dst));
    //Ptr<QbbNetDevice> devA = d.Get(2);
    char ipstring[16];
    sprintf(ipstring, "10.%d.%d.0", i / 254 + 1, i % 254 + 1);
    ipv4.SetBase(ipstring, "255.255.255.0");
    ipv4.Assign(d);
  }

  NodeContainer trace_nodes;
  for (uint32_t i = 0; i < trace_num; i++)
  {
    uint32_t nid;
    tracef >> nid;
    trace_nodes = NodeContainer(trace_nodes, nodes.Get(nid));
  }
  AsciiTraceHelper ascii;
  p2plink.EnableAscii(ascii.CreateFileStream(trace_output_file), trace_nodes);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Applications.");
  // uint32_t packetSize = packet_payload_size;
  Time interPacketInterval = Seconds(0.000005 / 2);

  for (uint32_t i = 0; i < flow_num; i++)
  {
    uint32_t src, dst, pg, MaxBytes, port;
    double start_time, stop_time;
    while (used_port[port = int(UniformVariable(0, 1).GetValue() * 40000)])
      continue;
    used_port[port] = true;
    flowf >> src >> dst >> pg >> MaxBytes >> start_time >> stop_time;
    std::cout << "src:" << src << "  dst:" << dst << "  pg:" << pg << "  maxP:" << MaxBytes << "  st:" << start_time << "  et:" << stop_time << std::endl;
    // NS_ASSERT(nodes.Get(src)->GetNodeType() == 0 && nodes.Get(dst)->GetNodeType() == 0);
    Ptr<Ipv4> ipv4 = nodes.Get(dst)->GetObject<Ipv4>();
    Ipv4Address serverAddress = ipv4->GetAddress(1, 0).GetLocal(); //GetAddress(0,0) is the loopback 127.0.0.1
    
    MpTcpPacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(dst));
    sinkApps.Start(Seconds(app_start_time));
    sinkApps.Stop(Seconds(app_stop_time));

    MpTcpBulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(serverAddress), port));
    source.SetAttribute("MaxBytes", UintegerValue(MaxBytes));
    ApplicationContainer sourceApps = source.Install(nodes.Get(src));
    sourceApps.Start(Seconds(start_time));
    sourceApps.Stop(Seconds(stop_time));
  }

  topof.close();
  flowf.close();
  tracef.close();
  tcpflowf.close();
  // Ipv4GlobalRouting::CalculateThroughput(2);
  std::cout << "Running Simulation.\n";
  fflush(stdout);
  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(simulator_stop_time));
  Simulator::Run(); 
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  endt = clock();
  std::cout << (double)(endt - begint) / CLOCKS_PER_SEC << "\n";
}
