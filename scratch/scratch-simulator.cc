#include <iostream>
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/command-line.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
using namespace ns3;
namespace {
class MyModel
{
public:
  void Start (void);
private:
  void HandleEvent (double eventValue);
};
void
MyModel::Start (void)
{
  Simulator::Schedule (Seconds (10.0),
                       &MyModel::HandleEvent,
                       this, Simulator::Now ().GetSeconds ());
}
void
MyModel::HandleEvent (double value)
{
  std::cout << "Member method received event at "
            << Simulator::Now ().GetSeconds ()
            << "s started at " << value << "s" << std::endl;
}
static void
ExampleFunction (MyModel *model)
{
  std::cout << "ExampleFunction received event at "
            << Simulator::Now ().GetSeconds () << "s" << std::endl;
  model->Start ();
}

static void
ExampleFunction2 (MyModel *model)
{
  std::cout << "ExampleFunction 2 received event at "
            << Simulator::Now ().GetSeconds () << "s" << std::endl;
  model->Start ();
}

static void
RandomFunction (void)
{
  std::cout << "RandomFunction received event at "
            << Simulator::Now ().GetSeconds () << "s" << std::endl;
}
static void
CancelledEvent (void)
{
  std::cout << "I should never be called... " << std::endl;
}
}  // unnamed namespace
int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  MyModel model;
  Ptr<UniformRandomVariable> v = CreateObject<UniformRandomVariable> ();
  v->SetAttribute ("Min", DoubleValue (10));
  v->SetAttribute ("Max", DoubleValue (20));
  Simulator::Schedule (Seconds (10.0), &ExampleFunction, &model);
  Simulator::Schedule (Seconds (5.0), &ExampleFunction2, &model);
  Simulator::Schedule (Seconds (v->GetValue ()), &RandomFunction);
  EventId id = Simulator::Schedule (Seconds (30.0), &CancelledEvent);
  Simulator::Cancel (id);
  Simulator::Run ();
  Simulator::Destroy ();
}