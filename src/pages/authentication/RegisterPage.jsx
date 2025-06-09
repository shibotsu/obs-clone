import { useState, useRef, useEffect } from "react";
import { useNavigate, Link } from "react-router-dom";
import { TextField, Stack, PrimaryButton } from "@fluentui/react";
import { CheckmarkFilled, ErrorCircleRegular } from "@fluentui/react-icons";
import { Text, Spinner } from "@fluentui/react-components";
import { DatePicker } from "@fluentui/react-datepicker-compat";
import "./Auth.css";
import authUseStyles from "./AuthUseStyles";

const RegisterPage = () => {
  const classes = authUseStyles();
  const [username, setUsername] = useState("");
  const [validUsername, setValidUsername] = useState(false);

  const [password, setPassword] = useState("");
  const [validPassword, setValidPassword] = useState(false);

  const [email, setEmail] = useState("");

  const [confirmPassword, setConfirmPassword] = useState("");
  const [birthday, setBirthday] = useState(null);
  const [error, setError] = useState("");
  const [isLoading, setIsLoading] = useState(false);

  // regexes for checking the username/password validity
  const isValidUsername = /^[0-9A-Za-z]{4,16}$/;
  const isValidPassword = /^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[\W_]).{8,32}$/;

  let navigate = useNavigate();
  const userRef = useRef();
  const emailRef = useRef();
  const passwordRef = useRef();

  const handleSubmit = async (e) => {
    e.preventDefault();
    setIsLoading(true);

    if (!password || !confirmPassword || !username || !email || !birthday) {
      setError("All fields are required.");
      return;
    }

    if (password !== confirmPassword) {
      setError("Passwords do not match.");
      setPassword("");
      setConfirmPassword("");
      if (passwordRef.current) {
        passwordRef.current.focus();
      }
      return;
    }

    if (!email.includes("@")) {
      setError("Please enter a valid email.");
      if (emailRef.current) {
        emailRef.current.focus();
      }

      return;
    }

    const formattedBirthday = formatBirthday(birthday);

    const requestData = {
      username: username,
      email: email,
      password: password,
      birthday: formattedBirthday,
    };

    try {
      const response = await fetch("http://157.230.16.67:8000/api/register", {
        method: "POST",
        headers: {
          "Content-Type": "aplication/json",
        },
        body: JSON.stringify(requestData),
      });

      if (!response.ok) {
        const errorData = await response.json();
        setError(errorData.message || "Register failed");
        return;
      }

      const data = await response.json();

      navigate("/login");
    } catch (error) {
      setError("Register failed: " + error);
    } finally {
      setIsLoading(false);
    }

    console.log(formatBirthday(birthday));

    navigate("/login");
  };

  const formatBirthday = (birthdayValue) => {
    const myArray = birthdayValue.toString().split(" ");
    return myArray[1] + " " + myArray[2] + " " + myArray[3];
  };

  const onDateChange = (date) => {
    setBirthday(date);
  };

  // on loading the page focus on the username/email field
  useEffect(() => {
    if (userRef.current) {
      userRef.current.focus();
    }
  }, []);

  // check if the username is valid
  useEffect(() => {
    const result = isValidUsername.test(username);
    setValidUsername(result);
  }, [username]);
  // check if password is valid
  useEffect(() => {
    const result = isValidPassword.test(password);
    setValidPassword(result);
  }, [password]);
  // render username field icon
  const renderIcon = (isValid, value) => {
    if (value.length === 0) return null;
    return isValid > 0 ? (
      <CheckmarkFilled className={classes.iconCheckmark} />
    ) : (
      <ErrorCircleRegular className={classes.iconError} />
    );
  };

  const renderConfirmPasswordIcon = () => {
    if (confirmPassword.length === 0) return null;
    return confirmPassword === password ? (
      <CheckmarkFilled className={classes.iconCheckmark} />
    ) : (
      <ErrorCircleRegular className={classes.iconError} />
    );
  };

  const today = new Date();

  return (
    <div>
      <form onSubmit={handleSubmit} className={classes.loginContainer}>
        <Stack tokens={{ childrenGap: 10 }} className={classes.stack}>
          <TextField
            componentRef={userRef}
            className={classes.textfield}
            label="Username"
            value={username}
            onChange={(e, newValue) => setUsername(newValue || "")}
            autoComplete="off"
            aria-invalid={validUsername ? "false" : "true"}
            aria-describedby="uidnote"
            errorMessage={
              username && !validUsername
                ? "Username must be 4-16 alphanumerical characters."
                : ""
            }
            onRenderSuffix={() => renderIcon(validUsername, username)}
            styles={{
              fieldGroup: { height: "37px !important" },
              suffix: {
                backgroundColor: "transparent",
              },
            }}
            required
          />
          <TextField
            componentRef={emailRef}
            className={classes.textfield}
            label="Email"
            value={email}
            onChange={(e, newValue) => setEmail(newValue || "")}
            styles={{
              fieldGroup: { height: "37px !important" },
            }}
            required
          />
          <TextField
            className="textfield"
            label="Password"
            type="password"
            value={password}
            componentRef={passwordRef}
            onChange={(e, newValue) => setPassword(newValue || "")}
            aria-invalid={validPassword ? "false" : "true"}
            errorMessage={
              password && !validPassword ? (
                <>
                  Password must be 8-32 characters. <br />
                  Must have at least 1 uppercase and 1 lowercase letter. <br />
                  Must have at least 1 number and 1 special character."
                </>
              ) : (
                ""
              )
            }
            onRenderSuffix={() => renderIcon(validPassword, password)}
            styles={{
              fieldGroup: { height: "37px !important" },
              suffix: {
                backgroundColor: "transparent",
              },
            }}
            required
          />
          <TextField
            className={classes.textfield}
            label="Confirm Password"
            type="password"
            value={confirmPassword}
            onChange={(e, newValue) => setConfirmPassword(newValue || "")}
            onRenderSuffix={renderConfirmPasswordIcon}
            errorMessage={
              confirmPassword.length > 0 && confirmPassword !== password
                ? "Must be the same as the password."
                : ""
            }
            styles={{
              fieldGroup: { height: "37px !important" },
              suffix: {
                backgroundColor: "transparent",
              },
            }}
            required
          />
          <DatePicker
            className={classes.datePicker}
            label="Enter your birthday"
            placeholder="Select a date..."
            value={birthday}
            onSelectDate={onDateChange}
            maxDate={today}
            required
          />
          {error && <p style={{ color: "red" }}>{error}</p>}
          {isLoading ? (
            <Spinner />
          ) : (
            <PrimaryButton
              text="Sign Up"
              type="submit"
              className="custom-primary-button"
              disabled={!(validUsername && validPassword)}
            />
          )}
          <div className={classes.alreadyRegistered}>
            <Text>Already registered?</Text>
            <Link className={classes.link} to={"/login"}>
              Sign In
            </Link>
          </div>
        </Stack>
      </form>
    </div>
  );
};

export default RegisterPage;
