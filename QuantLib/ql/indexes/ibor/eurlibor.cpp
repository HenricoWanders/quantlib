/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/indexes/ibor/eurlibor.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    namespace {

        BusinessDayConvention eurliborConvention(const Period& p) {
            switch (p.units()) {
              case Days:
              case Weeks:
                return Following;
              case Months:
              case Years:
                return ModifiedFollowing;
              default:
                QL_FAIL("invalid time units");
            }
        }

        bool eurliborEOM(const Period& p) {
            switch (p.units()) {
              case Days:
              case Weeks:
                return false;
              case Months:
              case Years:
                return true;
              default:
                QL_FAIL("invalid time units");
            }
        }

    }

    EURLibor::EURLibor(const Period& tenor,
                       const Handle<YieldTermStructure>& h)
    : IborIndex("EURLibor", tenor, 2, EURCurrency(),
                // http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412 :
                // JoinBusinessDays is the fixing calendar for
                // all indexes but o/n
                JointCalendar(UnitedKingdom(UnitedKingdom::Exchange),
                              TARGET(),
                              JoinBusinessDays),
                eurliborConvention(tenor), eurliborEOM(tenor),
                Actual360(), h),
      target_(TARGET()) {}

    Date EURLibor::valueDate(const Date& fixingDate) const {

        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");

        // http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412 :
        // In the case of EUR the Value Date shall be two TARGET
        // business days after the Fixing Date.
        return target_.advance(fixingDate, fixingDays_, Days);
    }

    Date EURLibor::maturityDate(const Date& valueDate) const {
        // http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412 :
        // In the case of EUR only, maturity dates will be based on days in
        // which the Target system is open.
        if (endOfMonth() && target_.isEndOfMonth(valueDate)) {
            Date d = valueDate + tenor_;
            Date last = Date::endOfMonth(d);
            return target_.adjust(last, Preceding);
        } else {
            return target_.advance(valueDate, tenor_, convention_);
        }
    }

}
