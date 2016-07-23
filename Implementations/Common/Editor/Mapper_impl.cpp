#include "Mapper_impl.h"
#include "Curve_impl.h"

namespace impl
{

JamomaMapper::JamomaMapper(shared_ptr<Address> driverAddress,
                           shared_ptr<Address> drivenAddress,
                           const Value& drive) :
  JamomaTimeProcess(),
  mDriverAddress(driverAddress),
  mDrivenAddress(drivenAddress),
  mDrive(drive),
  mDriverValueObserved(false)
{}

JamomaMapper::JamomaMapper(const JamomaMapper& other) :
  JamomaTimeProcess()
{}

shared_ptr<Mapper> JamomaMapper::clone() const
{
  return make_shared<JamomaMapper>(*this);
}

JamomaMapper::~JamomaMapper()
{}

# pragma mark -
# pragma mark Execution

StateElement JamomaMapper::offset(TimeValue offset)
{
  if (parent->getRunning())
    throw runtime_error("parent time constraint is running");

  //! \todo return nothing
  return mMessageToSend;
}

StateElement JamomaMapper::state()
{
  auto& par = *parent;
  if (!par.getRunning())
    throw runtime_error("parent time constraint is not running");

  // if date hasn't been processed already
  TimeValue date = par.getDate();
  if (date != mLastDate)
  {
    mLastDate = date;

    if (mValueToMap.valid())
    {
      std::lock_guard<std::mutex> lock(mValueToMapMutex);

      // edit a Message handling the mapped value
      auto newval = computeValue(mValueToMap, mDrive);
      mMessageToSend = Message{mDrivenAddress, newval};

      // forget the former value
      mValueToMap.reset();
    }
  }

  return mMessageToSend;
}

# pragma mark -
# pragma mark Execution - Implementation specific

void JamomaMapper::start()
{
  // start driver address value observation
  if (!mDriverValueObserved)
  {
    mDriverValueCallbackIndex = mDriverAddress->addCallback(
          [this] (const OSSIA::Value& val) {
      driverValueCallback(val);
    });
    mDriverValueObserved = true;
    auto def_val = mDriverAddress->cloneValue();
    driverValueCallback(def_val);
  }
}

void JamomaMapper::stop()
{
  // stop driver address value observation
  if (mDriverValueObserved)
  {
    mDriverAddress->removeCallback(mDriverValueCallbackIndex);
    mDriverValueObserved = false;
  }
}

void JamomaMapper::pause()
{}

void JamomaMapper::resume()
{}

# pragma mark -
# pragma mark Accessors

const shared_ptr<Address> JamomaMapper::getDriverAddress() const
{
  return mDriverAddress;
}

const shared_ptr<Address> JamomaMapper::getDrivenAddress() const
{
  return mDrivenAddress;
}

const Value& JamomaMapper::getDriving() const
{
  return mDrive;
}

Value JamomaMapper::computeValue(const Value& driver, const Value& drive)
{
  switch (drive.getType())
  {
    case Type::BEHAVIOR :
    {
      auto& b = drive.get<Behavior>();

      auto base_curve = b.value.get();
      auto t = base_curve->getType();
      switch(t.first)
      {
        case OSSIA::CurveSegmentType::FLOAT:
        {
          auto& val = driver.get<Float>();
          switch(t.second)
          {
            case OSSIA::CurveSegmentType::FLOAT:
            {
              auto curve = static_cast<JamomaCurve<float, float>*>(base_curve);
              return Float{curve->valueAt(val.value)};
            }
            case OSSIA::CurveSegmentType::INT:
            {
              auto curve = static_cast<JamomaCurve<float, int>*>(base_curve);
              return Int{curve->valueAt(val.value)};
            }
            case OSSIA::CurveSegmentType::BOOL:
            {
              auto curve = static_cast<JamomaCurve<float, bool>*>(base_curve);
              return Bool{curve->valueAt(val.value)};
            }
          }
        }
        case OSSIA::CurveSegmentType::INT:
        {
          auto& val = driver.get<Int>();
          switch(t.second)
          {
            case OSSIA::CurveSegmentType::FLOAT:
            {
              auto curve = static_cast<JamomaCurve<int, float>*>(base_curve);
              return Float{curve->valueAt(val.value)};
            }
            case OSSIA::CurveSegmentType::INT:
            {
              auto curve = static_cast<JamomaCurve<int, int>*>(base_curve);
              return Int{curve->valueAt(val.value)};
            }
            case OSSIA::CurveSegmentType::BOOL:
            {
              auto curve = static_cast<JamomaCurve<int, bool>*>(base_curve);
              return Bool{curve->valueAt(val.value)};
            }
          }
        }
        case OSSIA::CurveSegmentType::BOOL:
        {
          auto& val = driver.get<Bool>();
          switch(t.second)
          {
            case OSSIA::CurveSegmentType::FLOAT:
            {
              auto curve = static_cast<JamomaCurve<bool, float>*>(base_curve);
              return Float{curve->valueAt(val.value)};
            }
            case OSSIA::CurveSegmentType::INT:
            {
              auto curve = static_cast<JamomaCurve<bool, int>*>(base_curve);
              return Int{curve->valueAt(val.value)};
            }
            case OSSIA::CurveSegmentType::BOOL:
            {
              auto curve = static_cast<JamomaCurve<bool, bool>*>(base_curve);
              return Bool{curve->valueAt(val.value)};
            }
          }
        }
      }
      throw runtime_error("none handled driver value type");

      break;
    }

    case Type::TUPLE :
    {
      auto& t_drive = drive.get<Tuple>();

      if (driver.getType() == Type::TUPLE)
      {
        auto& t_driver = driver.get<Tuple>();

        vector<Value> t_value;
        t_value.reserve(t_drive.value.size());
        auto it_driver = t_driver.value.begin();

        for (const auto & e_drive : t_drive.value)
        {
          if (it_driver == t_driver.value.end())
            break;

          t_value.push_back(computeValue(*it_driver, e_drive));
          it_driver++;
        }

        return Tuple{std::move(t_value)};
      }
    }

    default :
    {
      throw runtime_error("none handled drive value type");
    }
  }

  throw runtime_error("none handled drive value type");
}

void JamomaMapper::driverValueCallback(const Value& value)
{
  std::lock_guard<std::mutex> lock(mValueToMapMutex);

  mValueToMap = value;
}
}
